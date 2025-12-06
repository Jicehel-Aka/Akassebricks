#include "driver/i2c_master.h"
#include "common.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/soc_caps.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "TAS2505_rehs.h"
#include "expander.h"

static void delay(uint32_t u32_ms)
{
    vTaskDelay( u32_ms / portTICK_PERIOD_MS);
}

#define I2C_PORT_NUM_0 ((i2c_port_num_t)0)
i2c_master_bus_config_t i2c_mst_config = {
    .i2c_port = I2C_NUM_0    ,
    .sda_io_num = EXPANDER_I2C_SDA,
    .scl_io_num = EXPANDER_I2C_SCL,
    .clk_source = I2C_CLK_SRC_DEFAULT,
    .glitch_ignore_cnt = 7,
    .flags.enable_internal_pullup = true,
};

i2c_master_bus_handle_t bus_handle;

i2c_device_config_t dev_cfg0 = {
    .dev_addr_length = I2C_ADDR_BIT_LEN_7,
    .device_address = EXPANDER_I2C_ADDRESS0,
    .scl_speed_hz = 400000,
};
i2c_master_dev_handle_t dev_handle0;

i2c_device_config_t dev_cfg1 = {
    .dev_addr_length = I2C_ADDR_BIT_LEN_7,
    .device_address = EXPANDER_I2C_ADDRESS1,
    .scl_speed_hz = 400000,
};
i2c_master_dev_handle_t dev_handle1;

i2c_device_config_t dev_cfg_audio = {
    .dev_addr_length = I2C_ADDR_BIT_LEN_7,
    .device_address = AUDIO_AMP_I2C_ADDRESS,
    .scl_speed_hz = 400000,
};
i2c_master_dev_handle_t dev_handle_audio;



static uint8_t u8_expander_out_data = 0;
void expander_write(uint8_t u8_data)
{
    u8_expander_out_data = u8_data |= EXPANDER_KEY; // Key inputs must be stay HIGH
    esp_err_t ret = i2c_master_transmit(dev_handle0, &u8_expander_out_data, sizeof(u8_expander_out_data), -1);
    if(ret)
        printf( "I2C wite return %d\n", ret );
}

void expander_lcd_reset(uint8_t state)
{
    if ( state )
        expander_write( u8_expander_out_data | EXPANDER_LCD_nRESET );
    else    
        expander_write( u8_expander_out_data & ~EXPANDER_LCD_nRESET );
}


void expander_audio_amplifier_reset(uint8_t state)
{
    if ( state )
        expander_write( u8_expander_out_data | EXPANDER_AMP_nRESET );
    else    
        expander_write( u8_expander_out_data & ~EXPANDER_AMP_nRESET );
}
#if (BOARD_VERSION < 4) // hard connected on V1.4+
void expander_lcd_cs(uint8_t state)
{
    if ( state )
        expander_write( u8_expander_out_data | EXPANDER_LCD_nCS );
    else    
        expander_write( u8_expander_out_data & ~EXPANDER_LCD_nCS );
}
#else
void expander_lcd_rd(uint8_t state)
{
    if ( state )
        expander_write( u8_expander_out_data | EXPANDER_LCD_nRD );
    else    
        expander_write( u8_expander_out_data & ~EXPANDER_LCD_nRD );
}
#endif

uint16_t expander_read()
{
    uint8_t u8_d1 = 0x55;
    esp_err_t ret1 = i2c_master_receive(dev_handle1, &u8_d1, sizeof(u8_d1), -1);
    uint8_t u8_d0 = 0x55;
    esp_err_t ret0 = i2c_master_receive(dev_handle0, &u8_d0, sizeof(u8_d0), -1);
    uint16_t u16_data = u8_d0 + 256*(uint16_t)u8_d1;
    u16_data ^= EXPANDER_KEY_RUN; // Run key active high => active low
    u16_data ^= EXPANDER_KEY;     // all key active low => active high
    if ( ret0 || ret1 )
    {
        printf( "I2C read return %d %d\n", ret0, ret1 );
        return 0;
    }
//    printf( "I2C read return %d %d 0x%04X\n", ret0, ret1, u16_data );
    return u16_data;
}

void expander_power_off()
{
    while ( expander_read() & EXPANDER_KEY_RUN )
        ; // wait user release button to assure don't restart if user keep RUN key down after power off


    expander_write( u8_expander_out_data &= ~EXPANDER_AMP_nRESET );
    expander_write( u8_expander_out_data |= EXPANDER_KEY_RUN );
    delay(250);
    expander_write( u8_expander_out_data &= ~EXPANDER_OUT_ENA_3V3 );
}

void audio_amp_write(uint8_t u8_reg, uint8_t u8_data)
{
    uint8_t u8_datareg[2] = { u8_reg, u8_data };
//    u8_data |= EXPANDER_KEY; // Key inputs must be stay HIGH
    esp_err_t ret = i2c_master_transmit( dev_handle_audio, u8_datareg, sizeof(u8_datareg), -1);
    if(ret)
        printf( "I2C Audio wite return %d\n", ret );
}

uint8_t audio_amp_read( uint8_t u8_reg )
{
    uint8_t u8_data = 0;
    esp_err_t ret = i2c_master_transmit_receive( dev_handle_audio, &u8_reg, sizeof(u8_reg), &u8_data, sizeof(u8_data), -1 );
    if ( ret )
    {
        printf( "ERR : I2C Audio read return %d\n", ret );
        return 0;
    }
    printf( "I2C Audio read reg 0x%02X return 0x%02x\n", u8_reg, u8_data );
    return u8_data;
}

void audio_set_volume( uint8_t u8_volume )
{
    u8_volume = 127 - u8_volume/2; // 0  volume max, 116 = volume min
    audio_amp_write( AUDIO_AMP_REG_PAGE, 1 ); // switch page 1
    if( u8_volume >= 116 ) // mute
    {
        audio_amp_write( AUDIO_AMP_P1_SPK_VOL, 0x7F ); 
        audio_amp_write( AUDIO_AMP_P1_HP_SPK_VOL, 0x7F ); 
    }
    else
    {
        audio_amp_write( AUDIO_AMP_P1_SPK_VOL, u8_volume ); 
        audio_amp_write( AUDIO_AMP_P1_HP_SPK_VOL, u8_volume ); 
    }
}


void audio_set_vibrator( uint8_t u8_on )
{
    audio_amp_write( AUDIO_AMP_REG_PAGE, 0 ); // W 30 00 00

    if ( u8_on ) // toggle on
        audio_amp_write( AUDIO_AMP_P0_GPIO_CTRL, 0b00001101 ); // Set as HIGH
    else
        audio_amp_write( AUDIO_AMP_P0_GPIO_CTRL, 0b00001100 ); // Set as LOW
}

void test_expander()
{
    for (int i = 0 ; ; i++ )
//    for (int i = 0 ; i < 10 ; i++ )
//    if(0)
    {
        uint16_t u16_exp = expander_read();
        printf( "%4d EXP = 0x%04X\n", i, u16_exp );
        
        if ( u16_exp & EXPANDER_KEY_RUN ) // RUN/STOP key
        {
            printf( "Set to power down\n" );
            expander_write(0);
        }
        
        if ( u16_exp & EXPANDER_KEY_RUN )   printf( "KEY RUN\n" );
        if ( u16_exp & EXPANDER_KEY_MENU )  printf( "KEY MENU\n" );
        if ( u16_exp & EXPANDER_KEY_L1 )    printf( "KEY L1\n" );
        if ( u16_exp & EXPANDER_KEY_R1 )    printf( "KEY R1\n" );
        if ( u16_exp & EXPANDER_KEY_UP )    printf( "KEY UP\n" );
        if ( u16_exp & EXPANDER_KEY_DOWN )  printf( "KEY DOWN\n" );
        if ( u16_exp & EXPANDER_KEY_LEFT )  printf( "KEY LEFT\n" );
        if ( u16_exp & EXPANDER_KEY_RIGHT ) printf( "KEY RIGHT\n" );
        if ( u16_exp & EXPANDER_KEY_A )     printf( "KEY A\n" );
        if ( u16_exp & EXPANDER_KEY_B )     printf( "KEY B\n" );
        if ( u16_exp & EXPANDER_KEY_C )     printf( "KEY C\n" );
        if ( u16_exp & EXPANDER_KEY_D )     printf( "KEY D\n" );
        
        delay(100);
    }   
}

int expander_init()
{
//    printf( "Initialize I2C master bus... " ); delay(250);
    esp_err_t ret = i2c_new_master_bus(&i2c_mst_config, &bus_handle);
    printf( "Return %d\n", ret );

//    printf( "Initialize I2C Slave 0... " );delay(250);
    esp_err_t ret0 = i2c_master_bus_add_device(bus_handle, &dev_cfg0, &dev_handle0);
    printf( "Return %d\n", ret0 );
    
//    printf( "Initialize I2C Slave 1... " );delay(250);
    esp_err_t ret1 = i2c_master_bus_add_device(bus_handle, &dev_cfg1, &dev_handle1);
    printf( "Return %d\n", ret1 );

//    printf( "Initialize I2C Audio... " );delay(250);
    esp_err_t ret2 = i2c_master_bus_add_device(bus_handle, &dev_cfg_audio, &dev_handle_audio);
    printf( "Return %d\n", ret2 );

//    printf( " write I2C expander ... " );delay(250);
    #if (BOARD_VERSION < 4) // RD# en I2C expander
    expander_write( EXPANDER_OUT_ENA_3V3 | EXPANDER_AMP_nRESET | EXPANDER_LCD_nRESET | EXPANDER_LCD_nCS );
    #else
    expander_write( EXPANDER_OUT_ENA_3V3 | EXPANDER_AMP_nRESET | EXPANDER_LCD_nRESET | EXPANDER_LCD_nRD );
    #endif
    expander_write( EXPANDER_OUT_ENA_3V3   );
    delay(100);
    expander_write( EXPANDER_OUT_ENA_3V3 | EXPANDER_LCD_nRESET  );
    delay(100);
    printf( "Done\n" );
//    test_expander();
    if ( ret | ret0 | ret1 )
        return -1; // fail

    return 0;
}

#include "driver/ledc.h"

/*
int lcd_init_pwm(uint8_t u8_duty)
{
    ledcAttach( PWM_LCD_GPIO, PWM_LCD_FREQUENCY, PWM_LCD_RESOLUTION );
    ledcWrite( PWM_LCD_GPIO, u8_duty );
    ledc_set_freq();
    ledc_set_duty();
    ledc_set_pin();
}
*/
#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO          (PWM_LCD_GPIO) // Define the output GPIO
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_8_BIT // Set duty resolution to 13 bits
#define LEDC_DUTY               (4096) // Set duty to 50%. (2 ** 13) * 50% = 4096
#define LEDC_FREQUENCY          (PWM_LCD_FREQUENCY) // Frequency in Hertz. Set frequency at 4 kHz

/* Warning:
 * For ESP32, ESP32S2, ESP32S3, ESP32C3, ESP32C2, ESP32C6, ESP32H2, ESP32P4 targets,
 * when LEDC_DUTY_RES selects the maximum duty resolution (i.e. value equal to SOC_LEDC_TIMER_BIT_WIDTH),
 * 100% duty cycle is not reachable (duty cannot be set to (2 ** SOC_LEDC_TIMER_BIT_WIDTH)).
 */

void lcd_init_pwm()
{
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .duty_resolution  = LEDC_DUTY_RES,
        .timer_num        = LEDC_TIMER,
        .freq_hz          = LEDC_FREQUENCY,  // Set output frequency at 4 kHz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LEDC_OUTPUT_IO,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));


}

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"


void lcd_update_pwm(uint8_t u8_duty)
{
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, u8_duty));
    // Update duty to apply the new value
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
}

adc_oneshot_unit_handle_t adc1_handle;
const adc_oneshot_unit_init_cfg_t init_config1 = {
    .unit_id = ADC_UNIT_1,
};

const adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };

    // ADC1 Calibration Init 
adc_cali_handle_t adc1_cali_chan0_handle = NULL;
adc_cali_handle_t adc1_cali_chan1_handle = NULL;
adc_cali_handle_t adc1_cali_chan2_handle = NULL;

/*---------------------------------------------------------------
        ADC Calibration
---------------------------------------------------------------*/
const static char *TAG = "ADC";
static bool example_adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            .chan = channel,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Line Fitting");
        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

    *out_handle = handle;
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Calibration Success");
    } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    } else {
        ESP_LOGE(TAG, "Invalid arg or no memory");
    }

    return calibrated;
}

#if 0
static void example_adc_calibration_deinit(adc_cali_handle_t handle)
{
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    ESP_LOGI(TAG, "deregister %s calibration scheme", "Curve Fitting");
    ESP_ERROR_CHECK(adc_cali_delete_scheme_curve_fitting(handle));

#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    ESP_LOGI(TAG, "deregister %s calibration scheme", "Line Fitting");
    ESP_ERROR_CHECK(adc_cali_delete_scheme_line_fitting(handle));
#endif
}
#endif

int adc_init()
{
    esp_err_t ret = adc_oneshot_new_unit(&init_config1, &adc1_handle);
    printf( "adc_oneshot_new_unit return %d\n", ret );

        //-------------ADC1 Config---------------//
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC1_CHANNEL_BATTERY, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC1_CHANNEL_JOYX, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC1_CHANNEL_JOYY, &config));

    bool do_calibration1_chan0 = example_adc_calibration_init( ADC_UNIT_1, ADC1_CHANNEL_BATTERY, ADC_ATTEN_DB_12, &adc1_cali_chan0_handle);
    bool do_calibration1_chan1 = example_adc_calibration_init( ADC_UNIT_1, ADC1_CHANNEL_JOYX, ADC_ATTEN_DB_12, &adc1_cali_chan1_handle);    
    bool do_calibration1_chan2 = example_adc_calibration_init( ADC_UNIT_1, ADC1_CHANNEL_JOYY, ADC_ATTEN_DB_12, &adc1_cali_chan2_handle);    
    printf( "ADC calibration ch0 return %s\n", do_calibration1_chan0?"Success":"Fail" );
    printf( "ADC calibration ch1 return %s\n", do_calibration1_chan1?"Success":"Fail" );
    printf( "ADC calibration ch2 return %s\n", do_calibration1_chan2?"Success":"Fail" );


    return 0;
}

    // return battery voltage as mV : 2500 ~ 4200
int adc_read_vbatt()
{
    int adc_raw, voltage;
    ESP_ERROR_CHECK( adc_oneshot_read(adc1_handle, ADC1_CHANNEL_BATTERY, &adc_raw) );
//    printf("RAW %d\n", adc_raw);
    adc_cali_raw_to_voltage( adc1_cali_chan0_handle, adc_raw, &voltage );
    return 2*voltage;
}

    // return battery voltage as % : 0 ~ 100
int adc_read_vbatt_percent()
{
    int adc_raw, voltage;
    ESP_ERROR_CHECK( adc_oneshot_read(adc1_handle, ADC1_CHANNEL_BATTERY, &adc_raw) );
//    printf("RAW %d\n", adc_raw);
    adc_cali_raw_to_voltage( adc1_cali_chan0_handle, adc_raw, &voltage );
    voltage*=2; // convert to battery voltage with 1:2 divider
    if ( voltage > 4000 )
        return 100; // full
    if ( voltage < 3300 )
        return 0; // empty

    return 100*(voltage - 3300)/(4000-3300);   
}

int adc_read_joyx()
{
    int adc_raw, voltage;
    ESP_ERROR_CHECK( adc_oneshot_read(adc1_handle, ADC1_CHANNEL_JOYX, &adc_raw) );
//    printf("RAW %d\n", adc_raw);
    adc_cali_raw_to_voltage( adc1_cali_chan1_handle, adc_raw, &voltage );
    return voltage;
}

int adc_read_joyy()
{
    int adc_raw, voltage;
    ESP_ERROR_CHECK( adc_oneshot_read(adc1_handle, ADC1_CHANNEL_JOYY, &adc_raw) );
//    printf("RAW %d\n", adc_raw);
    adc_cali_raw_to_voltage( adc1_cali_chan2_handle, adc_raw, &voltage );
    return voltage;
}
