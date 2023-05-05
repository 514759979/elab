# edf
-----
eLab Device Framework. eLab设备框架。

-----
EDF V2.0 应该如何实现。

### 设备类与驱动类分开实现
以串口举例说明，设备类和驱动类分开实现如下。

``` C
typedef struct elab_serial
{
    elab_device_t super;

    osMessageQueueId_t queue_rx;
    osMessageQueueId_t queue_tx;
    bool is_sending;
} elab_serial_t;

typedef struct elab_serial_driver
{
    elab_driver_t super;

    const struct elab_serial_ops *ops;
    elab_serial_attr_t attr;
} elab_serial_driver_t;

typedef struct elab_serial_ops
{
    elab_err_t (* enable)(elab_serial_t *serial, bool status);
    int32_t (* read)(elab_serial_t *serial, void *buffer, uint32_t size);
    int32_t (* write)(elab_serial_t *serial, const void *buffer, uint32_t size);
    void (* set_tx)(elab_serial_t *serial, bool status);
    elab_err_t (* config)(elab_serial_t *serial, elab_serial_config_t *config);
} elab_serial_ops_t;
```

### 设备类与驱动类分开注册


``` C
/* 注册设备，对设备进行初始化，并注册一下空驱动，使之在运行时不会崩溃。 */
void elab_serial_register(elab_serial_t *serial, const char *name);
void elab_serial_driver_register(elab_serial_driver_t *driver, elab_serial_ops_t *ops,
                                    elab_serial_attr_t *attr,
                                    void *user_data);
```

### 设备与驱动，如何产生联系？

在设备框架的基类里，实现两个函数。

``` C
void elab_device_add_driver(const char *name, elab_driver_t *driver);
void elab_device_driver_enable(const char *name, const char *driver);
```

好处是，一个设备可以有多个驱动，可以使用shell切换，可以使用脚本指定随意切换驱动。只要支持过的驱动，都可以编译在程序里，用脚本配置一下，底层的程序就可以不变。

