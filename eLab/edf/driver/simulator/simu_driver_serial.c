/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* include ------------------------------------------------------------------ */
#include "simu_driver_serial.h"
#include "edf_simu_config.h"
#include "elib_hash_table.h"
#include "cmsis_os.h"
#include "elab_def.h"
#include "elab_serial.h"
#include "elab_common.h"
#include "elab_assert.h"
#include "elab_def.h"

ELAB_TAG("SimuDriverSerial");

/* private define ----------------------------------------------------------- */
enum simu_serial_mode
{
    SIMU_SERAIL_MODE_SINGLE = 0,
    SIMU_SERAIL_MODE_PAIR,
    SIMU_SERAIL_MODE_MASTER,
    SIMU_SERAIL_MODE_SLAVE,
};

/* private typedef ---------------------------------------------------------- */
typedef struct simu_serial
{
    const char *name;
    uint8_t mode;
    bool enable;
    bool tx_mode;
    
    osMutexId_t mutex;
    osMessageQueueId_t queue_tx;
    osMessageQueueId_t queue_rx;
    elab_serial_t serial;

    struct simu_serial *partner;
    struct simu_serial *slave[SIMU_SERAIL_SLAVE_NUM_MAX];
    uint32_t count_slave;
} simu_serial_t;

/* private function prototype ----------------------------------------------- */
static elab_err_t _enable(elab_serial_t *serial, bool status);
static int32_t _read(elab_serial_t *serial, void *buffer, uint32_t size);
static int32_t _write(elab_serial_t *serial, const void *buffer, uint32_t size);
static elab_err_t _config(elab_serial_t *serial, elab_serial_config_t *config);
static void _set_tx(elab_serial_t *serial, bool status);

/* private variables -------------------------------------------------------- */
elib_hash_table_t *ht_simu= NULL;

static elab_serial_ops_t simu_serial_ops =
{
    .enable = _enable,
    .read = _read,
    .write = _write,
    .set_tx = _set_tx,
    .config = _config,
};

static const osMutexAttr_t mutex_attr_simu_serial =
{
    "MutexSimuSerial",
    osMutexRecursive | osMutexPrioInherit,
    NULL,
    0U
};

/* public function ---------------------------------------------------------- */
static simu_serial_t *_simu_serial_new(const char *name)
{
    elab_err_t ret = ELAB_OK;

    if (ht_simu == NULL)
    {
        ht_simu = elib_hash_table_new(SIMU_HASH_TABLE_SIZE);
        assert(ht_simu != NULL);
    }

    assert(!elib_hash_table_existent(ht_simu, (char *)name));

    simu_serial_t *serial = elab_malloc(sizeof(simu_serial_t));
    assert(serial != NULL);
    serial->enable = false;

    serial->mode = SIMU_SERAIL_MODE_SINGLE;
    serial->queue_rx = osMessageQueueNew(SIMU_SERIAL_BUFFER_MAX, 1, NULL);
    serial->queue_tx = osMessageQueueNew(SIMU_SERIAL_BUFFER_MAX, 1, NULL);
    serial->mutex = osMutexNew(&mutex_attr_simu_serial);
    assert(serial->queue_rx != NULL &&
                serial->queue_tx != NULL &&
                serial->mutex != NULL);

    elib_hash_table_add(ht_simu, (char *)name, (void *)serial);
    elab_serial_register(&serial->serial, name, &simu_serial_ops, NULL, serial);

    return serial;
}

void simu_serial_new(const char *name)
{
    _simu_serial_new(name);
}

void simu_serial_new_pair(const char *name_one, const char *name_two)
{
    simu_serial_t *serial_one = NULL;
    simu_serial_t *serial_two = NULL;

    serial_one = _simu_serial_new(name_one);
    assert(serial_one != NULL);
    serial_one->mode = SIMU_SERAIL_MODE_PAIR;

    serial_two = _simu_serial_new(name_two);
    assert(serial_two != NULL);
    serial_two->mode = SIMU_SERAIL_MODE_PAIR;
    
    serial_one->partner = serial_two;
    serial_two->partner = serial_one;
}

void simu_serial_new_add_slave(const char *name_master, const char *name_slave)
{
    assert(false);
}

void simu_serial_make_rx_data(const char *name, void *buffer, uint32_t size)
{
    simu_serial_t *serial = elib_hash_table_get(ht_simu, (char *)name);
    assert(serial != NULL);
    assert(serial->mode == SIMU_SERAIL_MODE_SINGLE);

    /* TODO Add osDelay function to simulate the serial port transmitting
            process. Start one specific thread to send rx data.
    */
    osStatus_t ret = osOK;
    uint8_t *buff = (uint8_t *)buffer;
    for (uint32_t i = 0; i < size; i ++)
    {
        ret = osMessageQueuePut(serial->queue_rx, &buff[i], 0, 0);
        assert(ret == osOK);
    }
}

void simu_serial_make_rx_data_delay(const char *name,
                                    void *buffer, uint32_t size,
                                    uint32_t delay_ms)
{
    osDelay(delay_ms);
    simu_serial_make_rx_data(name, buffer, size);
}

uint32_t simu_serial_read_tx_data(const char *name,
                                    void *buffer, uint32_t size,
                                    uint32_t timeout_ms)
{
    int32_t ret = ELAB_ERR_TIMEOUT;
    osStatus_t ret_queue = osOK;

    simu_serial_t *serial = elib_hash_table_get(ht_simu, (char *)name);
    assert(serial != NULL);
    assert(serial->mode == SIMU_SERAIL_MODE_SINGLE);

    uint8_t *buff = (uint8_t *)buffer;
    for (uint32_t i = 0; i < size; i ++)
    {
        ret_queue = osMessageQueueGet(serial->queue_tx, &buff[i], NULL, timeout_ms);
        if (ret_queue == osOK)
        {
            ret = (ret == ELAB_ERR_TIMEOUT) ? 1 : (ret + 1);
        }
        else if (ret_queue == osErrorTimeout)
        {
            break;
        }
        else
        {
            assert(false);
        }
    }

    return ret;
}

/* private functions -------------------------------------------------------- */
static elab_err_t _enable(elab_serial_t *serial, bool status)
{
    osStatus_t ret = osOK;

    simu_serial_t *simu_serial = container_of(serial, simu_serial_t, serial);
    ret = osMutexAcquire(simu_serial->mutex, osWaitForever);
    assert(ret == osOK);

    simu_serial->enable = status;

    ret = osMutexRelease(simu_serial->mutex);
    assert(ret == osOK);

    return ELAB_OK;
}

static int32_t _read(elab_serial_t *serial, void *pbuf, uint32_t size)
{
    osStatus_t ret = osOK;

    simu_serial_t *simu_serial = container_of(serial, simu_serial_t, serial);
    assert(simu_serial->enable);

    uint32_t read_cnt = 0;
    uint8_t *buffer = (uint8_t *)pbuf;
    for (uint32_t i = 0; i < size; i ++)
    {
        ret = osMessageQueueGet(simu_serial->queue_rx, &buffer[i], NULL, osWaitForever);
        assert(ret == osOK);
        read_cnt ++;
    }

exit:
    return read_cnt;
}

static int32_t _write(elab_serial_t *serial, const void *pbuf, uint32_t size)
{
    osStatus_t ret = osOK;

    simu_serial_t *simu_serial = container_of(serial, simu_serial_t, serial);
    assert(simu_serial->enable);

    if (simu_serial->mode == SIMU_SERAIL_MODE_SINGLE)
    {
        ret = osMutexAcquire(simu_serial->mutex, osWaitForever);
        assert(ret == osOK);

        /* Write the buffer data into message queue. */
        uint8_t *buffer = (uint8_t *)pbuf;
        for (uint32_t i = 0; i < size; i ++)
        {
            ret = osMessageQueuePut(simu_serial->queue_tx, &buffer[i], 0, 0);
            assert(ret == osOK);
        }

        ret = osMutexRelease(simu_serial->mutex);
        assert(ret == osOK);
    }
    else if (simu_serial->mode == SIMU_SERAIL_MODE_PAIR)
    {
        simu_serial_t *partner = simu_serial->partner;
        ret = osMutexAcquire(partner->mutex, osWaitForever);
        assert(ret == osOK);

        /* Write the buffer data into message queue. */
        uint8_t *buffer = (uint8_t *)pbuf;
        for (uint32_t i = 0; i < size; i ++)
        {
            ret = osMessageQueuePut(partner->queue_rx, &buffer[i], 0, 0);
            assert(ret == osOK);
        }

        ret = osMutexRelease(partner->mutex);
        assert(ret == osOK);
    }

exit:
    return size;
}

static void _set_tx(elab_serial_t *serial, bool status)
{

}

static elab_err_t _config(elab_serial_t *serial, elab_serial_config_t *config)
{
    osStatus_t ret_queue = osOK;

    return ret_queue;
}

/* ----------------------------- end of file -------------------------------- */
