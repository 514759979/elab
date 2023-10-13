
/* include ------------------------------------------------------------------ */
#include <string.h>
#include "ebutton.h"
#include "../../common/elab_assert.h"
#include "../../common/elab_export.h"
#include "../../os/cmsis_os.h"

ELAB_TAG("eButton");

#define BUTTON_CLICK_TIME                   (300)
#define BUTTON_LONG_PRE_TIME                (1000)

enum button_state
{
    BUTTON_STATE_IDLE = 0,
    BUTTON_STATE_CLICK_PRE = 1,
    BUTTON_STATE_CLICK_END = 2,
    BUTTON_STATE_DOUBLE_CLICK_PRE = 102,
    BUTTON_STATE_DOUBLE_CLICK_END = 104,
    BUTTON_STATE_LONG_PRESS_PRE = 5,
};

static void _button_fsm(elab_button_action_t *action, uint32_t time_passed);
static void _mutex_ebutton_new(void);

static osMutexId_t mutex_ebutton = NULL;
static elab_button_t *button_list = NULL;
static elab_button_action_t *action_list = NULL;
static uint32_t time_backup = 0;

/**
 * The edf global mutex attribute.
 */
static const osMutexAttr_t _mutex_attr_ebutton =
{
    .name = "mutex_ebutton",
    .attr_bits = osMutexPrioInherit | osMutexRecursive,
    .cb_mem = NULL,
    .cb_size = 0,
};

/* public function ---------------------------------------------------------- */
void elab_button_init(elab_button_t *const me,
                        const char *name, elab_button_status_t ops)
{
    _mutex_ebutton_new();

}

void elab_button_attach(elab_button_action_t *action)
{
    _mutex_ebutton_new();
}


/* private function --------------------------------------------------------- */
static void _button_poll(void)
{
    osStatus_t ret_os = osOK;

#if !defined(__linux__) && !defined(_WIN32)
    /* Button debounce */
    ret_os = osMutexAcquire(mutex_ebutton, osWaitForever);
    elab_assert(ret_os == osOK);

    elab_button_t *button = button_list;
    while (button != NULL)
    {
        button->flag <<= 1;
        if (button->ops())
        {
            button->flag |= 0x01;
        }
        else
        {
            button->flag &= 0xfe;
        }

        if ((button->flag & 0x0f) == 0x0f)
        {
            button->status = true;
        }
        if ((button->flag | 0xf0) == 0xf0)
        {
            button->status = false;
        }

        button = button->next;
    }

    ret_os = osMutexRelease(mutex_ebutton);
    elab_assert(ret_os == osOK);
#endif

    ret_os = osMutexAcquire(mutex_ebutton, osWaitForever);
    elab_assert(ret_os == osOK);

    uint32_t time_passed = 0;
    if (osKernelGetTickCount() > time_backup)
    {
        time_passed = osKernelGetTickCount() - time_backup;
    }
    else
    {
        time_passed = UINT32_MAX - time_backup + 1 + osKernelGetTickCount();
    }

    elab_button_action_t *action = action_list;
    while (action != NULL)
    {
        _button_fsm(action, time_passed);
        action = action->next;
    }

    time_backup = osKernelGetTickCount();

    ret_os = osMutexRelease(mutex_ebutton);
    elab_assert(ret_os == osOK);
}
POLL_EXPORT(_button_poll, 3);

static void _mutex_ebutton_new(void)
{
    if (mutex_ebutton == NULL)
    {
        mutex_ebutton = osMutexNew(&_mutex_attr_ebutton);
        elab_assert(mutex_ebutton != NULL);

        time_backup = osKernelGetTickCount();
    }
}

static bool _button_status(const char *name)
{
    bool ret = true;

    if (name != NULL)
    {
        elab_button_t *button = button_list;
        bool existent = false;
        while (button != NULL)
        {
            if (strcmp(name, button->name) == 0)
            {
                ret = button->status;
                existent = true;
                break;
            }

            button = button->next;
        }
        elab_assert_name(existent, name);
    }

    return ret;
}

static void _button_fsm(elab_button_action_t *action, uint32_t time_passed)
{
    bool pressed = false;
    if (_button_status(action->name_button0) &&
        _button_status(action->name_button1) &&
        _button_status(action->name_button2))
    {
        action->time_count += time_passed;
        pressed = true;
    }
    else
    {
        action->time_count = 0;
    }

    /*-----------------State machine-------------------*/
    switch (action->state)
    {
    case BUTTON_STATE_IDLE:
        if (pressed)   //start press down
        {
            if (action->event_type == ELAB_BUTTON_EVT_PRESS_DOWN)
            {
                action->cb();
            }
            action->time = 0;
            action->repeat = 1;
            action->state = BUTTON_STATE_CLICK_PRE;
        }
        break;

    case BUTTON_STATE_CLICK_PRE:
        /* released press up. */
        if (!pressed)
        { 
            if (action->event_type == ELAB_BUTTON_EVT_LONGPRESS_START)
            {
                action->cb();
            }
            action->time = 0;
            action->state = BUTTON_STATE_CLICK_END;
        }
        else if (action->time > BUTTON_LONG_PRE_TIME)
        {
            if (action->event_type == ELAB_BUTTON_EVT_LONGPRESS_START)
            {
                action->cb();
            }
            action->state = BUTTON_STATE_LONG_PRESS_PRE;
        }
        break;

    case BUTTON_STATE_CLICK_END:
        /* Press down again. */
        if (pressed)
        {
            if (action->event_type == ELAB_BUTTON_EVT_PRESS_DOWN)
            {
                action->cb();
            }
            action->repeat++;
            if (action->event_type == ELAB_BUTTON_EVT_REPEAT)
            {
                action->cb(); // repeat hit
            }
            action->time = 0;
            action->state = BUTTON_STATE_DOUBLE_CLICK_PRE;
        }
        /* Released timeout. */
        else if (action->time > BUTTON_CLICK_TIME)
        {
            if (action->repeat == BUTTON_STATE_CLICK_PRE)
            {
                if (action->event_type == ELAB_BUTTON_EVT_CLICK)
                {
                    action->cb();
                }
            }
            else if (action->repeat == BUTTON_STATE_CLICK_END)
            {
                if (action->event_type == ELAB_BUTTON_EVT_DOUBLE_CLICK)
                {
                    action->cb();
                }
            }
            action->state = BUTTON_STATE_IDLE;
        }
        break;

    case BUTTON_STATE_DOUBLE_CLICK_PRE:
        /* released press up. */
        if (!pressed)
        { 
            if (action->event_type == ELAB_BUTTON_EVT_PRESS_UP)
            {
                action->cb();
            }
            
            if (action->time < BUTTON_CLICK_TIME)
            {
                action->time = 0;
                action->state = BUTTON_STATE_CLICK_END; //repeat press
            }
            else
            {
                action->state = BUTTON_STATE_IDLE;
            }
        }
        /* Long press up. */
        else if (action->time > BUTTON_CLICK_TIME)
        { 
            action->state = BUTTON_STATE_IDLE;
        }
        break;

    case BUTTON_STATE_LONG_PRESS_PRE:
        if (pressed)
        {
            /* continue hold trigger. */
            if (action->event_type == ELAB_BUTTON_EVT_LONGPRESS_HOLD)
            {
                action->cb();
            }
        }
        /* Releasd. */
        else
        { 
            if (action->event_type == ELAB_BUTTON_EVT_PRESS_UP)
            {
                action->cb();
            }
            action->state = BUTTON_STATE_IDLE;      /* Reset */
        }
        break;

    default:
        action->state = BUTTON_STATE_IDLE;          /* Reset */
        break;
    }
}

/* ----------------------------- end of file -------------------------------- */
