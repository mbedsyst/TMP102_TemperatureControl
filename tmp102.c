#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/regmap.h>

#include "tmp102.h"

// Structure to hold Device specific data
struct tmp102_data
{
	struct i2c_client *client;
};

// Function to Read Temperature data register
static int tmp102_read_temp(struct device *dev, long *temp)
{
	pr_info("TMP102: Temperature Read function called.\n");

	struct tmp102_data *data = dev_get_drvdata(dev);
	struct i2c_client *client = data->client;
	int ret;
	u16 val;
	
	ret = i2c_smbus_read_word_data(client, TMP102_TEMP_REG);
	if(ret < 0)
	{
		pr_err("TMP102: Temperature read failed.\n");
		return ret;
	}

	val = be16_to_cpu(ret);
	*temp = (val >> 4) * 625;

	return 0;
}

// Function to read specific sysfs attribute
static int tmp102_read(struct device *dev, enum hwmon_sensor_types type, u32 attr, int channel, long *val)
{
	pr_info("TMP102: Attribute Read function called.\n");

	switch(attr)
	{
		case hwmon_temp_input:
			return tmp102_read_temp(dev, val);

		default:
			pr_err("TMP102: Unsupported attributed called.\n");
			return -EOPNOTSUPP;
	}
}

// 
static umode_t tmp102_is_visible(const void *data, enum hwmon_sensor_types type, u32 attr, int channel)
{
	pr_info("TMP102: HWMON Visibility function called.\n");
	return 0;
}

// Structure to hold Device operations
static const struct hwmon_ops tmp102_hwmon_ops =
{
	.is_visible	= tmp102_is_visible,
	.read		= tmp102_read,
};

// Structure to hold Device channel info
static const struct hwmon_channel_info *tmp102_channel_info[] =
{
	HWMON_CHANNEL_INFO(temp, HWMON_T_INPUT),
	NULL
};

// Structure to hold Device info
static const struct hwmon_chip_info tmp102_chip_info =
{
	.ops	= &tmp102_hwmon_ops,
	.info	= tmp102_channel_info,
};

// Executed when I2C device is connected
static int tmp102_probe(struct i2c_client *client)
{
    pr_info("TMP102: I2C Probe function called.\n");
  
    struct device *dev = &client->dev;
    struct tmp102_data *data;
    struct device *hwmon_dev;

    data = devm_kzalloc(dev, sizeof(struct tmp102_data), GFP_KERNEL);
    if (!data) 
    {
	pr_err("TMP102: Device Data memory allocation failed.\n");
        return -ENOMEM;
    }
    data->client = client;
    dev_set_drvdata(dev, data);

    hwmon_dev = hwmon_device_register_with_info(dev, "tmp102", data, &tmp102_chip_info, NULL);
    if (IS_ERR(hwmon_dev)) 
    {
	pr_err("TMP102: Device registration with HWMON subsystem failed.\n");
        return PTR_ERR(hwmon_dev);
    }

    return 0;
}

// Executed when I2C device is removed
static void tmp102_remove(struct i2c_client *client)
{
	pr_info("TMP102: I2C Remove function called.\n");
	struct device *dev = &client->dev;
	hwmon_device_unregister(dev);
}

static const struct of_device_id tmp102_of_match[] =
{
	{ .compatible = "ti, tmp102" },
	{ }	
};
MODULE_DEVICE_TABLE(of, tmp102_of_match);

// Structure to hold Device ID
static const struct i2c_device_id tmp102_id[] =
{
	{"tmp102", 0},
	{ }
};
// To auto-load the driver when device is detected
MODULE_DEVICE_TABLE(i2c, tmp102_id);

// Represents the I2C device driver
static struct i2c_driver tmp102_driver =
{
	.driver	=
	{
		.name 		= "tmp102",
		.owner 		= THIS_MODULE,
		.of_match_table = tmp102_of_match,
	},
	.probe		= tmp102_probe,
	.remove 	= tmp102_remove,
	.id_table	= tmp102_id,
};

// Module Init and Exit function wrapper
module_i2c_driver(tmp102_driver);

// Module Metadata macros
MODULE_AUTHOR("mbedsyst");
MODULE_DESCRIPTION("TMP102 Temperature Sensor Driver");
MODULE_LICENSE("GPL");
