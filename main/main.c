/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include <driver/i2c.h>
#include "driver/gpio.h"

#define PIN_SDA 1
#define PIN_CLK 2
#define I2C_ADDRESS 0x36 // I2C address of MPU6050

static char tag[] = "MAX17260";

// #undef ESP_ERROR_CHECK
// #define ESP_ERROR_CHECK(x)   do { esp_err_t rc = (x); if (rc != ESP_OK) { ESP_LOGE("err", "esp_err_t = %d", rc); assert(0 && #x);} } while(0);

void max_17260_init(void *params)
{
    uint8_t StatusPOR =0x00;
    uint8_t FSAT = 0x00;
    uint8_t HibCFG = 0x00;
    uint8_t Model_CFG = 0x00;
    uint8_t status = 0x00;
    uint16_t Soc = 0x00;
    uint16_t Repcap=0x00;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
	conf.sda_io_num = PIN_SDA;
	conf.scl_io_num = PIN_CLK;
	conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
	conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
	conf.master.clk_speed = 100000;
	conf.clk_flags = 0;
    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &conf));
	ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));
    i2c_cmd_handle_t cmd;

    cmd = i2c_cmd_link_create();
	ESP_ERROR_CHECK(i2c_master_start(cmd));
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_WRITE, 1));
	i2c_master_write_byte(cmd, 0x00, 1);
	ESP_ERROR_CHECK(i2c_master_stop(cmd));
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);

    cmd = i2c_cmd_link_create();
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_READ, 1));
    ESP_ERROR_CHECK(i2c_master_read_byte(cmd, &StatusPOR, 1));
    ESP_ERROR_CHECK(i2c_master_stop(cmd));
    ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS));
    i2c_cmd_link_delete(cmd);

    StatusPOR &= 0x0002;

    if(StatusPOR == 0)
    {
        StatusPOR &= 0x0002;

        ////////////////////////////////////////////////////////////////
        //Read and write for Soc from 0x06
        cmd = i2c_cmd_link_create();
		ESP_ERROR_CHECK(i2c_master_start(cmd));
		ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_WRITE, 1));
		ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x06, 1));
		ESP_ERROR_CHECK(i2c_master_stop(cmd));
		ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS));
		i2c_cmd_link_delete(cmd);
        
        cmd = i2c_cmd_link_create();
        ESP_ERROR_CHECK(i2c_master_start(cmd));
        ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_READ, 1));
        ESP_ERROR_CHECK(i2c_master_read_byte(cmd, Soc, 1));
        ESP_ERROR_CHECK(i2c_master_stop(cmd));
        ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS));
        i2c_cmd_link_delete(cmd);
        printf("%u\n",Soc);


    }
    else
    {
        cmd = i2c_cmd_link_create();
	    ESP_ERROR_CHECK(i2c_master_start(cmd));
	    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_WRITE, 1));
	    i2c_master_write_byte(cmd, 0x3D, 1);
	    ESP_ERROR_CHECK(i2c_master_stop(cmd));
	    i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS);
	    i2c_cmd_link_delete(cmd);
    
        cmd = i2c_cmd_link_create();
        ESP_ERROR_CHECK(i2c_master_start(cmd));
        ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_READ, 1));
        ESP_ERROR_CHECK(i2c_master_read_byte(cmd, &FSAT, 1));
        ESP_ERROR_CHECK(i2c_master_stop(cmd));
        ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS));
        i2c_cmd_link_delete(cmd);

        while(FSAT&1)
        {
            vTaskDelay(10/portTICK_PERIOD_MS);
            cmd = i2c_cmd_link_create();
	        ESP_ERROR_CHECK(i2c_master_start(cmd));
	        ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_WRITE, 1));
	        i2c_master_write_byte(cmd, 0x3D, 1);
	        ESP_ERROR_CHECK(i2c_master_stop(cmd));
	        i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS);
	        i2c_cmd_link_delete(cmd);
        
            cmd = i2c_cmd_link_create();
            ESP_ERROR_CHECK(i2c_master_start(cmd));
            ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_READ, 1));
            ESP_ERROR_CHECK(i2c_master_read_byte(cmd, &FSAT, 1));
            ESP_ERROR_CHECK(i2c_master_stop(cmd));
            ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS));
            i2c_cmd_link_delete(cmd);
        }
        
        
        cmd = i2c_cmd_link_create();
	    ESP_ERROR_CHECK(i2c_master_start(cmd));
	    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_WRITE, 1));
	    i2c_master_write_byte(cmd, 0xBA, 1);
	    ESP_ERROR_CHECK(i2c_master_stop(cmd));
	    i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS);
	    i2c_cmd_link_delete(cmd);
    
        cmd = i2c_cmd_link_create();
        ESP_ERROR_CHECK(i2c_master_start(cmd));
        ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_READ, 1));
        ESP_ERROR_CHECK(i2c_master_read_byte(cmd, &HibCFG, 1));
        ESP_ERROR_CHECK(i2c_master_stop(cmd));
        ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS));
        i2c_cmd_link_delete(cmd);

        ////////////////////////////////////////////////////////////////////////
        // Mode 1
        cmd = i2c_cmd_link_create();
	    ESP_ERROR_CHECK(i2c_master_start(cmd));
	    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_WRITE, 1));
	    i2c_master_write_byte(cmd, 0x60, 1);
        ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x90, 1));
	    ESP_ERROR_CHECK(i2c_master_stop(cmd));
	    i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS);
	    i2c_cmd_link_delete(cmd);
    



        ////////////////////////////////////////////////////////////////////////
        // Mode 2
        cmd = i2c_cmd_link_create();
	    ESP_ERROR_CHECK(i2c_master_start(cmd));
	    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_WRITE, 1));
	    i2c_master_write_byte(cmd, 0xBA, 1);
        ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x00, 1));
	    ESP_ERROR_CHECK(i2c_master_stop(cmd));
	    i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS);
	    i2c_cmd_link_delete(cmd);





        ////////////////////////////////////////////////////////////////////////
        // Mode 3
        cmd = i2c_cmd_link_create();
	    ESP_ERROR_CHECK(i2c_master_start(cmd));
	    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_WRITE, 1));
	    i2c_master_write_byte(cmd, 0x60, 1);
        ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x00, 1));
	    ESP_ERROR_CHECK(i2c_master_stop(cmd));
	    i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS);
	    i2c_cmd_link_delete(cmd);
        
        //DesignCap
        cmd = i2c_cmd_link_create();
	    ESP_ERROR_CHECK(i2c_master_start(cmd));
	    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_WRITE, 1));
	    i2c_master_write_byte(cmd, 0x18, 1);
        ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x0BB8, 1));
	    ESP_ERROR_CHECK(i2c_master_stop(cmd));
	    i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS);
	    i2c_cmd_link_delete(cmd);

        //IchgTerm
        cmd = i2c_cmd_link_create();
	    ESP_ERROR_CHECK(i2c_master_start(cmd));
	    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_WRITE, 1));
	    i2c_master_write_byte(cmd, 0x1E, 1);
        ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x0640, 1));
	    ESP_ERROR_CHECK(i2c_master_stop(cmd));
	    i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS);
	    i2c_cmd_link_delete(cmd);
    
        //VEmpty
        cmd = i2c_cmd_link_create();
	    ESP_ERROR_CHECK(i2c_master_start(cmd));
	    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_WRITE, 1));
	    i2c_master_write_byte(cmd, 0x3A, 1);
        ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0xA561, 1));
	    ESP_ERROR_CHECK(i2c_master_stop(cmd));
	    i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS);
	    i2c_cmd_link_delete(cmd);

        //Charge Voltage is less than or equal 4.2 so i did for 0x8000 if greater write 0x8400 in this register
        cmd = i2c_cmd_link_create();
	    ESP_ERROR_CHECK(i2c_master_start(cmd));
	    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_WRITE, 1));
	    i2c_master_write_byte(cmd, 0xDB, 1);
        ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x8000, 1));//if greater than 4.2 volt write 0x8400
	    ESP_ERROR_CHECK(i2c_master_stop(cmd));
	    i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS);
	    i2c_cmd_link_delete(cmd);

        //Model_CFG
        cmd = i2c_cmd_link_create();
		ESP_ERROR_CHECK(i2c_master_start(cmd));
		ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_WRITE, 1));
		ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0xDB, 1));
		ESP_ERROR_CHECK(i2c_master_stop(cmd));
		ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS));
		i2c_cmd_link_delete(cmd);
        
        cmd = i2c_cmd_link_create();
        ESP_ERROR_CHECK(i2c_master_start(cmd));
        ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_READ, 1));
        ESP_ERROR_CHECK(i2c_master_read_byte(cmd, &Model_CFG, 1));
        ESP_ERROR_CHECK(i2c_master_stop(cmd));
        ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS));
        i2c_cmd_link_delete(cmd);
        
        while(Model_CFG & 0x8000)
        {
            vTaskDelay(10/portTICK_PERIOD_MS);
            cmd = i2c_cmd_link_create();
		    ESP_ERROR_CHECK(i2c_master_start(cmd));
		    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_WRITE, 1));
		    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0xDB, 1));
		    ESP_ERROR_CHECK(i2c_master_stop(cmd));
		    ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS));
		    i2c_cmd_link_delete(cmd);
            

            cmd = i2c_cmd_link_create();
            ESP_ERROR_CHECK(i2c_master_start(cmd));
            ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_READ, 1));
            ESP_ERROR_CHECK(i2c_master_read_byte(cmd, &Model_CFG, 1));
            ESP_ERROR_CHECK(i2c_master_stop(cmd));
            ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS));
            i2c_cmd_link_delete(cmd);

        }
       
        cmd = i2c_cmd_link_create();
	    ESP_ERROR_CHECK(i2c_master_start(cmd));
	    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_WRITE, 1));
	    i2c_master_write_byte(cmd, 0xBA, 1);
        ESP_ERROR_CHECK(i2c_master_write_byte(cmd, HibCFG, 1));//if greater than 4.2 volt write 0x8400
	    ESP_ERROR_CHECK(i2c_master_stop(cmd));
	    i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS);
	    i2c_cmd_link_delete(cmd);


//////////////////////////////////////////////////////////////////////////////////////
        //status read write
        cmd = i2c_cmd_link_create();
		ESP_ERROR_CHECK(i2c_master_start(cmd));
		ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_WRITE, 1));
		ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x00, 1));
		ESP_ERROR_CHECK(i2c_master_stop(cmd));
		ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS));
		i2c_cmd_link_delete(cmd);
        
        cmd = i2c_cmd_link_create();
        ESP_ERROR_CHECK(i2c_master_start(cmd));
        ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_READ, 1));
        ESP_ERROR_CHECK(i2c_master_read_byte(cmd, &status, 1));
        ESP_ERROR_CHECK(i2c_master_stop(cmd));
        ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS));
        i2c_cmd_link_delete(cmd);
        

///////////////////////////////////////////////////////////////////////////////////////////////

       



    }

       
        cmd = i2c_cmd_link_create();
		ESP_ERROR_CHECK(i2c_master_start(cmd));
		ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_WRITE, 1));
		ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x00, 1));
		ESP_ERROR_CHECK(i2c_master_stop(cmd));
		ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS));
		i2c_cmd_link_delete(cmd);
        
        cmd = i2c_cmd_link_create();
        ESP_ERROR_CHECK(i2c_master_start(cmd));
        ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_READ, 1));
		ESP_ERROR_CHECK(i2c_master_read_byte(cmd, &status, 1));
        ESP_ERROR_CHECK(i2c_master_stop(cmd));
        ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS));
        i2c_cmd_link_delete(cmd);
        

        cmd = i2c_cmd_link_create();
	    ESP_ERROR_CHECK(i2c_master_start(cmd));
	    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_WRITE, 1));
	    i2c_master_write_byte(cmd, 0x00, 1);
        ESP_ERROR_CHECK(i2c_master_write_byte(cmd, status & 0xFFFD, 1));
	    ESP_ERROR_CHECK(i2c_master_stop(cmd));
	    i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS);
	    i2c_cmd_link_delete(cmd);

    

    while(1)
    {
        // StatusPOR &= 0x0002;
        //printf("%u\n",StatusPOR);
        uint16_t test[2];
        uint16_t init_val;
        cmd = i2c_cmd_link_create();
		ESP_ERROR_CHECK(i2c_master_start(cmd));
		ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_WRITE, 1));
		ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x00, 2));
		ESP_ERROR_CHECK(i2c_master_stop(cmd));
		ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS));
		i2c_cmd_link_delete(cmd);
        
        cmd = i2c_cmd_link_create();
        ESP_ERROR_CHECK(i2c_master_start(cmd));
        ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_READ, 1));
		ESP_ERROR_CHECK(i2c_master_read_byte(cmd, test,   0));
		ESP_ERROR_CHECK(i2c_master_read_byte(cmd, test+1, 1));
        ESP_ERROR_CHECK(i2c_master_stop(cmd));
        ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS));
        i2c_cmd_link_delete(cmd);

        init_val = (test[1]<<8) | test[0];
        //printf("%u\n",init_val);

        ////////////////////////////////////////////////////////////////
        //Read and write for Soc from 0x06
        cmd = i2c_cmd_link_create();
		ESP_ERROR_CHECK(i2c_master_start(cmd));
		ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_WRITE, 1));
		ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x05, 1));
		ESP_ERROR_CHECK(i2c_master_stop(cmd));
		ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS));
		i2c_cmd_link_delete(cmd);
        
        cmd = i2c_cmd_link_create();
        ESP_ERROR_CHECK(i2c_master_start(cmd));
        ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_READ, 1));
        ESP_ERROR_CHECK(i2c_master_read_byte(cmd, &Soc, 1));
        ESP_ERROR_CHECK(i2c_master_stop(cmd));
        ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS));
        i2c_cmd_link_delete(cmd);
        //printf("%u\n",Soc);

        uint8_t data[4];
        cmd = i2c_cmd_link_create();
		ESP_ERROR_CHECK(i2c_master_start(cmd));
		ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_WRITE, 1));
		ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x06, 4));
		ESP_ERROR_CHECK(i2c_master_stop(cmd));
		ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS));
		i2c_cmd_link_delete(cmd);
        
        cmd = i2c_cmd_link_create();
        ESP_ERROR_CHECK(i2c_master_start(cmd));
        ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_READ, 1));
		ESP_ERROR_CHECK(i2c_master_read_byte(cmd, data,   0));
		ESP_ERROR_CHECK(i2c_master_read_byte(cmd, data+1, 0));
        ESP_ERROR_CHECK(i2c_master_read_byte(cmd, data+2,   0));
		ESP_ERROR_CHECK(i2c_master_read_byte(cmd, data+3, 1));
        ESP_ERROR_CHECK(i2c_master_stop(cmd));
        ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS));
        i2c_cmd_link_delete(cmd);

        Soc = (data[2]<<8) | data[3];
        
       //printf("charge: %u\n",Soc);
        printf("%u \n",data[1]);


        cmd = i2c_cmd_link_create();
		ESP_ERROR_CHECK(i2c_master_start(cmd));
		ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_WRITE, 1));
		ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x05, 1));
		ESP_ERROR_CHECK(i2c_master_stop(cmd));
		ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS));
		i2c_cmd_link_delete(cmd);
        
        cmd = i2c_cmd_link_create();
        ESP_ERROR_CHECK(i2c_master_start(cmd));
        ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_READ, 1));
		ESP_ERROR_CHECK(i2c_master_read_byte(cmd, &Repcap, 1));
        ESP_ERROR_CHECK(i2c_master_stop(cmd));
        ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS));
        i2c_cmd_link_delete(cmd);

       // printf("%u\n",Repcap);
        vTaskDelay(500/portTICK_PERIOD_MS);
    }


}

void app_main(void)
{
    xTaskCreate(max_17260_init, "max_17260_init",8192, NULL, 1, NULL);
    
}
