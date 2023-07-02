/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* include ------------------------------------------------------------------ */
#include "simu_serial.h"
#include "edf_simu_config.h"
#include "../../../elib/elib_hash_table.h"
#include "../../../RTOS/cmsis_os.h"
#include "../../../edf/normal/elab_serial.h"
#include "../../../common/elab_common.h"
#include "../../../common/elab_assert.h"
#include "../../../common/elab_def.h"

ELAB_TAG("SimuSerial");

/* private typedef ---------------------------------------------------------- */
typedef struct simu_serial
{
    const char *name;
    uint8_t mode;
    uint32_t baudrate;
    bool enable;
    
    osMutexId_t mutex;
    osMessageQueueId_t queue_tx;
    osMessageQueueId_t queue_rx;
    elab_serial_t serial;

    struct simu_serial *partner;
    struct simu_serial *master;
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
static simu_serial_t *_simu_serial_new(const char *name, uint8_t mode, uint32_t baudrate)
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
    serial->baudrate = baudrate;
    serial->name = name;

    serial->mode = mode;
    serial->queue_rx = osMessageQueueNew(SIMU_SERIAL_BUFFER_MAX, 1, NULL);
    serial->queue_tx = osMessageQueueNew(SIMU_SERIAL_BUFFER_MAX, 1, NULL);
    serial->mutex = osMutexNew(&mutex_attr_simu_serial);
    assert(serial->queue_rx != NULL &&
                serial->queue_tx != NULL &&
                serial->mutex != NULL);

    serial->master = NULL;
    serial->partner = NULL;
    for (uint32_t i = 0; i < SIMU_SERAIL_SLAVE_NUM_MAX; i ++)
    {
        serial->slave[i] = NULL;
    }
    serial->count_slave = 0;

    elib_hash_table_add(ht_simu, (char *)name, (void *)serial);
    elab_serial_register(&serial->serial, name, &simu_serial_ops, NULL, serial);

    return serial;
}

void simu_serial_new(const char *name, uint8_t mode, uint32_t baudrate)
{
    elab_assert(name != NULL);
    assert_name(mode < SIMU_SERIAL_MODE_MAX, name);

    _simu_serial_new(name, mode, baudrate);
}

void simu_serial_new_pair(const char *name_one, const char *name_two, uint32_t baudrate)
{
    simu_serial_t *serial_one = NULL;
    simu_serial_t *serial_two = NULL;

    serial_one = _simu_serial_new(name_one, SIMU_SERIAL_MODE_UART, baudrate);
    assert(serial_one != NULL);

    serial_two = _simu_serial_new(name_two, SIMU_SERIAL_MODE_UART, baudrate);
    assert(serial_two != NULL);
    
    serial_one->partner = serial_two;
    serial_two->partner = serial_one;
}

void simu_serial_add_slave(const char *name, const char *name_slave)
{
    /* Get the simulated serial master device.*/
    simu_serial_t *serial = elib_hash_table_get(ht_simu, (char *)name);
    elab_assert(serial != NULL);
    elab_assert(serial->mode == SIMU_SERIAL_MODE_485_M);
    elab_assert(serial->count_slave < SIMU_SERAIL_SLAVE_NUM_MAX);

    /* Get the simulated serial slave device.*/
    simu_serial_t *serial_s = elib_hash_table_get(ht_simu, (char *)name_slave);
    elab_assert(serial_s != NULL);
    elab_assert(serial_s->mode == SIMU_SERIAL_MODE_485_S);
    elab_assert(serial->baudrate == serial_s->baudrate);

    /* Add master to slave. */
    serial_s->master = serial;
    serial_s->partner = NULL;
    for (uint32_t i = 0; i < SIMU_SERAIL_SLAVE_NUM_MAX; i ++)
    {
        serial_s->slave[i] = NULL;
    }
    serial_s->count_slave = 0;

    /* Add slave to master. */
    serial->slave[serial->count_slave ++] = serial_s;
    serial->master = NULL;
    serial->partner = NULL;
}

void simu_serial_make_rx_data(const char *name, void *buffer, uint32_t size)
{
    simu_serial_t *serial = elib_hash_table_get(ht_simu, (char *)name);
    assert(serial != NULL);
    assert(serial->mode == SIMU_SERIAL_MODE_SINGLE);

    /* TODO Add osDelay function to simulate the serial port transmitting
            process. Start one specific thread to send rx data.
            It depends on thread-pool module.
    */
    osStatus_t ret = osOK;
    uint8_t *buff = (uint8_t *)buffer;
    for (uint32_t i = 0; i < size; i ++)
    {
        ret = osMessageQueuePut(serial->queue_rx, &buff[i], 0, osWaitForever);
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

    uint32_t read_cnt = 0;
    uint8_t *buffer = (uint8_t *)pbuf;
    buffer[0] = 0;
    for (uint32_t i = 0; i < size; i ++)
    {
        ret = osMessageQueueGet(simu_serial->queue_rx, &buffer[i], NULL, osWaitForever);
        if (ret != osOK)
        {
            break;
        }
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

    ret = osMutexAcquire(simu_serial->mutex, osWaitForever);
    assert(ret == osOK);

    if (simu_serial->mode == SIMU_SERIAL_MODE_SINGLE)
    {
        /* Write the buffer data into message queue. */
        uint8_t *buffer = (uint8_t *)pbuf;
        for (uint32_t i = 0; i < size; i ++)
        {
            ret = osMessageQueuePut(simu_serial->queue_tx, &buffer[i], 0, osWaitForever);
            assert(ret == osOK);
        }
    }
    else if (simu_serial->mode == SIMU_SERIAL_MODE_UART)
    {
        simu_serial_t *partner = simu_serial->partner;
        ret = osMutexAcquire(partner->mutex, osWaitForever);
        assert(ret == osOK);

        /* Write the buffer data into message queue. */
        uint8_t *buffer = (uint8_t *)pbuf;
        for (uint32_t i = 0; i < size; i ++)
        {
            ret = osMessageQueuePut(partner->queue_rx, &buffer[i], 0, osWaitForever);
            assert(ret == osOK);
        }

        ret = osMutexRelease(partner->mutex);
        assert(ret == osOK);
    }
    else if (simu_serial->mode == SIMU_SERIAL_MODE_485_M)
    {
        simu_serial_t *slave;
        for (uint32_t i = 0; i < simu_serial->count_slave; i ++)
        {
            slave = simu_serial->slave[i];
            ret = osMutexAcquire(slave->mutex, osWaitForever);
            assert(ret == osOK);

            /* Write the buffer data into message queue. */
            uint8_t *buffer = (uint8_t *)pbuf;
            for (uint32_t i = 0; i < size; i ++)
            {
                ret = osMessageQueuePut(slave->queue_rx, &buffer[i], 0, osWaitForever);
                assert(ret == osOK);
            }

            ret = osMutexRelease(slave->mutex);
            assert(ret == osOK);
        }
    }
    else if (simu_serial->mode == SIMU_SERIAL_MODE_485_S)
    {
        simu_serial_t *master = simu_serial->master;
        ret = osMutexAcquire(master->mutex, osWaitForever);
        assert(ret == osOK);

        /* Write the buffer data into message queue. */
        uint8_t *buffer = (uint8_t *)pbuf;
        for (uint32_t i = 0; i < size; i ++)
        {
            ret = osMessageQueuePut(master->queue_rx, &buffer[i], 0, osWaitForever);
            assert(ret == osOK);
        }

        ret = osMutexRelease(master->mutex);
        assert(ret == osOK);
    }

    ret = osMutexRelease(simu_serial->mutex);
    assert(ret == osOK);

exit:
    return size;
}

static void _set_tx(elab_serial_t *serial, bool status)
{
    simu_serial_t *simu_serial = container_of(serial, simu_serial_t, serial);
    assert(simu_serial->enable);
}

static elab_err_t _config(elab_serial_t *serial, elab_serial_config_t *config)
{
    osStatus_t ret_queue = osOK;

    return ret_queue;
}

/* ----------------------------- end of file -------------------------------- */
