/*
 * logger_file.c
 *
 *  Created on: Nov 12, 2020
 *      Author: Yolo
 */

/***********************************************************************************************************************
* Pragma directive
***********************************************************************************************************************/

/***********************************************************************************************************************
* Includes <System Includes>
***********************************************************************************************************************/

#include "logger_file.h"

#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs.h"
#include "esp_spiffs.h"
/***********************************************************************************************************************
* Macro definitions
***********************************************************************************************************************/
/***********************************************************************************************************************
* Typedef definitions
***********************************************************************************************************************/
#define FILE_PATH "/spiffs/"
/***********************************************************************************************************************
* Private global variables and functions
***********************************************************************************************************************/
static uint8_t check_map_size(void);
static bool logger_list_file(char *file_name, char *location);

esp_vfs_spiffs_conf_t conf = {
    .base_path = "/spiffs",
    .partition_label = NULL,
    .max_files = 5,
    .format_if_mount_failed = true};

static esp_err_t ret;
/***********************************************************************************************************************
* Exported global variables and functions (to be accessed by other files)
***********************************************************************************************************************/

/***********************************************************************************************************************
* Imported global variables and functions (from other files)
***********************************************************************************************************************/
/***********************************************************************************************************************
* Imported global variables and functions (from other files)
***********************************************************************************************************************/
void logger_LoadInfo_WifiStation(bool smart_config)
{
    //
    FILE *file;
    char buffer[256];
    memset(buffer, 0x00, sizeof(buffer));

    if (smart_config == true)
    {
        APP_LOGI("smart config save data");
        APP_LOGI("wifi_info.UserName: %s", wifi_info.wifi_name);
        APP_LOGI("wifi_info.Password: %s", wifi_info.wifi_pass);

        file = fopen("/spiffs/config.txt", "w");
        //ghi de gia tri smartconfig vao
        sprintf(buffer, "%s\n", wifi_info.wifi_name);
        fputs(buffer, file);

        memset(buffer, 0x00, sizeof(buffer));

        sprintf(buffer, "%s\n", wifi_info.wifi_pass);
        fputs(buffer, file);

        memset(buffer, 0x00, sizeof(buffer));
        fclose(file);
    }
    else
    {
        file = fopen("/spiffs/config.txt", "r");
        if (file == NULL)
        {
            sprintf(wifi_info.wifi_name, "%s", "thao123");
            sprintf(wifi_info.wifi_pass, "%s", "123456789");

            APP_LOGE("Error opening file");
            APP_LOGI("Create new file save param");
            //create new file and save old
            file = fopen("/spiffs/config.txt", "w");
            //save param
            //ghi de gia tri smartconfig vao
            sprintf(buffer, "%s\n", wifi_info.wifi_name);
            fputs(buffer, file);

            memset(buffer, 0x00, sizeof(buffer));

            sprintf(buffer, "%s\n", wifi_info.wifi_pass);
            fputs(buffer, file);

            fclose(file);
        }
        else
        {
            APP_LOGD("file da ton tai");
            // da ton tai file
            int length = 0;
            int number_elements = 0;
            // doc thong tin user/password
            char buffer_2[32];
            while (fgets(buffer, sizeof(buffer), file))
            {
            	switch (number_elements)
            	{
					case 0:
					{
		                length = strlen(buffer);
		                if (buffer[length - 1] == '\n')
		                    buffer[length - 1] = 0x00;
		                for (int var = 0; var < length; var++)
		                {
		                	buffer_2[var] = buffer[var];
						}
		                APP_LOGI("buffer = %s", buffer_2);
		                sprintf(wifi_info.wifi_name, "%s", buffer_2);
		                memset(buffer, 0x00, sizeof(buffer));
		                memset(buffer_2, 0x00, sizeof(buffer_2));
						break;
					}
					case 1:
					{
			            length = strlen(buffer);
			            if (buffer[length - 1] == '\n')
			                buffer[length - 1] = 0x00;
		                for (int var = 0; var < length; var++)
		                {
		                	buffer_2[var] = buffer[var];
						}
			            APP_LOGI("buffer = %s", buffer_2);
			            sprintf(wifi_info.wifi_pass, "%s", buffer_2);
			            memset(buffer, 0x00, sizeof(buffer));
						break;
					}
						break;
					default:
						break;
				}
            	number_elements++;
            }
            fclose(file);
        }
    }
}
/***********************************************************************************************************************
* Function Name:
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
bool logger_file_delete(char *file_name)
{
    bool status = false;
    char buffer_file_name[50];
    memset(buffer_file_name, 0x00, sizeof(buffer_file_name));

    sprintf(buffer_file_name, "%s%s.txt", FILE_PATH, file_name);
    if (logger_list_file(file_name, FILE_PATH) == true)
    {
        // TODO delete file here
        remove(buffer_file_name);
        APP_LOGD("remove done");
    }
    else
    {
        APP_LOGE("remove err");
        status = true; // not file
    }
    return status;
}

/***********************************************************************************************************************
* Function Name:
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/

void logger_file_init(void)
{
    ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            APP_LOGE("Failed to mount or format filesystem");
        }
        else if (ret == ESP_ERR_NOT_FOUND)
        {
            APP_LOGE("Failed to find SPIFFS partition");
        }
        else
        {
            APP_LOGE("Failed to initialize SPIFFS (%d)", ret);
        }
    }
}

/***********************************************************************************************************************
* static functions
***********************************************************************************************************************/
static uint8_t check_map_size(void)
{
    bool status = false;
    size_t total = 0, used = 0;
    ret = esp_spiffs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK)
    {
        status = false;
        APP_LOGD("Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    }
    else
    {
        status = true;
        APP_LOGD("Partition size: total: %d Mb, used: %d", total, used);
    }
    return status;
}
/***********************************************************************************************************************
* Function Name:
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
bool logger_read_number_line(char *file_name)
{
    FILE *fp;
    fp = fopen(file_name, "r");
    APP_LOGD("logger_read_number_line\n");
    char buf[256 + 1];
    int number_line = 0;

    if (fp == NULL)
    {
        APP_LOGE("Xay ra loi trong khi doc file");
        return (false);
    }
    while (fgets(buf, 1024, fp))
    {
        number_line++;
        APP_LOGD("data: %s", buf);
    }
    fclose(fp);
    APP_LOGD("number_line = %d", number_line);
    return true;
}
/***********************************************************************************************************************
* Function Name: logger_list_file
* Description  :
* Arguments    : file_name name of file need check
                 dir dir of file name
* Return Value : true/false
***********************************************************************************************************************/
static bool logger_list_file(char *file_name, char *location)
{
	bool check_map = check_map_size();
    DIR *d;
    struct dirent *dir;
    bool status = true;
    d = opendir(location);
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            APP_LOGD("**** %s\n", dir->d_name);
            if (file_name == NULL)
            {
            }
            else if (strcmp(file_name, dir->d_name) == 0)
            {
                APP_LOGD("**** file name %s have exist", dir->d_name);
                status = false;
            }
        }
        closedir(d);
    }
    return status;
}

/***********************************************************************************************************************
* End of file
***********************************************************************************************************************/
