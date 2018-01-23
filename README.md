# BH-NEO-6M_GPS
STM32F429+BH-NEO-6M_GPS+LCD
/**
  ******************************************************************************
  * @file    nmea_decode_test.c
  * @brief   测试NEMA解码库
  ******************************************************************************
  ******************************************************************************
  */ 
  
#include "stm32f4xx.h"
#include "./usart/bsp_debug_usart.h"
#include "./lcd/bsp_lcd.h"
#include "./gps/gps_config.h"
#include "ff.h"
#include "nmea/nmea.h"

  

#ifdef __GPS_LOG_FILE             //对SD卡上的gpslog.txt文件进行解码；（需要在sd卡上存放gpslog.txt文件）

FATFS fs;
FIL log_file;
FRESULT res; 
UINT br, bw;            					/* File R/W count */

/**
  * @brief  nmea_decode_test 解码GPS文件信息
  * @param  无
  * @retval 无
  */
void nmea_decode_test(void)
{
		double deg_lat;//转换成[degree].[degree]格式的纬度
		double deg_lon;//转换成[degree].[degree]格式的经度

    nmeaINFO info;          //GPS解码后得到的信息
    nmeaPARSER parser;      //解码时使用的数据结构  
    
    nmeaTIME beiJingTime;    //北京时间 

    char buff[2048];  
  
    char str_buff[100];
  
  	/*使用不透明前景层*/
    LCD_SetLayer(LCD_FOREGROUND_LAYER);  
    LCD_SetTransparency(0xff);
    
    LCD_Clear(LCD_COLOR_BLACK);	/* 清屏，显示全黑 */

    /*设置字体颜色及字体的背景颜色(此处的背景不是指LCD的背景层！注意区分)*/
    LCD_SetColors(LCD_COLOR_RED,LCD_COLOR_BLACK);
  
  	LCD_DisplayStringLine(LINE(1),(uint8_t *)" Wildfire STM32-F429");
    LCD_DisplayStringLine(LINE(2),(uint8_t *)"  GPS module");
    
  
    	/* 注册盘符 */
    res = f_mount(&fs,"0:",1);	
  
    if(res != FR_OK)
    {
      printf("\r\n！！SD卡挂载文件系统失败。(%d)，请给开发板接入SD卡\r\n",res);
      while(1);
    }

    /* 打开记录有GPS信息的文件 */
    res = f_open(&log_file,"0:gpslog.txt", FA_OPEN_EXISTING|FA_READ);

    if(!(res == FR_OK))
    {
        printf("\r\n打开gpslog.txt文件失败，请检查SD卡的根目录是否存放了gpslog.txt文件!\r\n");
        return ;      
    }
    

    /* 设置用于输出调试信息的函数 */
    nmea_property()->trace_func = &trace;
    nmea_property()->error_func = &error;
    nmea_property()->info_func = &gps_info;

    /* 初始化GPS数据结构 */
    nmea_zero_INFO(&info);
    nmea_parser_init(&parser);

    while(!f_eof(&log_file))
    {
      
        f_read(&log_file, &buff[0], 100, &br);

        /* 进行nmea格式解码 */
        nmea_parse(&parser, &buff[0], br, &info);
      
        /* 对解码后的时间进行转换，转换成北京时间 */
        GMTconvert(&info.utc,&beiJingTime,8,1);
        
        /* 输出解码得到的信息 */
				printf("\r\n时间%d-%02d-%02d,%d:%d:%d\r\n", beiJingTime.year+1900, beiJingTime.mon,beiJingTime.day,beiJingTime.hour,beiJingTime.min,beiJingTime.sec);
				
				//info.lat lon中的格式为[degree][min].[sec/60]，使用以下函数转换成[degree].[degree]格式
				deg_lat = nmea_ndeg2degree(info.lat);
				deg_lon = nmea_ndeg2degree(info.lon);
			
				printf("\r\n纬度：%f,经度%f\r\n",deg_lat,deg_lon);
        printf("\r\n海拔高度：%f 米 ", info.elv);
        printf("\r\n速度：%f km/h ", info.speed);
        printf("\r\n航向：%f 度", info.direction);
				
				printf("\r\n正在使用的GPS卫星：%d,可见GPS卫星：%d",info.satinfo.inuse,info.satinfo.inview);

				printf("\r\n正在使用的北斗卫星：%d,可见北斗卫星：%d",info.BDsatinfo.inuse,info.BDsatinfo.inview);
				printf("\r\nPDOP：%f,HDOP：%f，VDOP：%f",info.PDOP,info.HDOP,info.VDOP);
           
        /* 液晶输出 */
        
        /* 设置前景颜色（字体颜色）*/
        LCD_SetTextColor(LCD_COLOR_BLUE);
        
        LCD_DisplayStringLine(LINE(5),(uint8_t *)" GPS Info:");

        /* 设置前景颜色（字体颜色）*/
        LCD_SetTextColor(LCD_COLOR_WHITE);
        
        /* 显示时间日期 */
        sprintf(str_buff," Date:%04d/%02d/%02d Time:%02d:%02d:%02d", beiJingTime.year+1900, beiJingTime.mon,beiJingTime.day,beiJingTime.hour,beiJingTime.min,beiJingTime.sec);
        LCD_DisplayStringLine(LINE(6),(uint8_t *)str_buff);
      
        /* 纬度 经度*/
        sprintf(str_buff," latitude :%.6f ", deg_lat);
        LCD_DisplayStringLine(LINE(7),(uint8_t *)str_buff);
        
        sprintf(str_buff," longitude :%.6f",deg_lon);
        LCD_DisplayStringLine(LINE(8),(uint8_t *)str_buff);
        
        /* 正在使用的卫星 可见的卫星*/
        sprintf(str_buff," GPS  Satellite in use :%2d ", info.satinfo.inuse);
        LCD_DisplayStringLine(LINE(9),(uint8_t *)str_buff);    
        
       sprintf(str_buff," GPS Satellite in view :%2d", info.satinfo.inview);
        LCD_DisplayStringLine(LINE(10),(uint8_t *)str_buff);    

        /* 正在使用的卫星 可见的卫星*/
        sprintf(str_buff," BDS  Satellite in use :%2d ", info.BDsatinfo.inuse);
        LCD_DisplayStringLine(LINE(11),(uint8_t *)str_buff);    
        
       sprintf(str_buff," BDS Satellite in view :%2d", info.BDsatinfo.inview);
        LCD_DisplayStringLine(LINE(12),(uint8_t *)str_buff);    
        
        /* 海拔高度 */
        sprintf(str_buff," Altitude:%4.2f m", info.elv);
        LCD_DisplayStringLine(LINE(13),(uint8_t *)str_buff);
        
        /* 速度 */
        sprintf(str_buff," speed:%4.2f km/h", info.speed);
        LCD_DisplayStringLine(LINE(14),(uint8_t *)str_buff);
        
        /* 航向 */
        sprintf(str_buff," Track angle:%3.2f deg", info.direction);
        LCD_DisplayStringLine(LINE(15),(uint8_t *)str_buff);
        
        
	
	}

    f_lseek(&log_file, f_size(&log_file));

    /* 释放GPS数据结构 */
    nmea_parser_destroy(&parser);
  
    /* 关闭文件 */
    f_close(&log_file);
    
   
}
#else       //对GPS模块传回的信息进行解码

/**
  * @brief  nmea_decode_test 解码GPS模块信息
  * @param  无
  * @retval 无
  */
int nmea_decode_test(void)
{
		double deg_lat;//转换成[degree].[degree]格式的纬度
		double deg_lon;//转换成[degree].[degree]格式的经度

    nmeaINFO info;          //GPS解码后得到的信息
    nmeaPARSER parser;      //解码时使用的数据结构  
    uint8_t new_parse=0;    //是否有新的解码数据标志
  
    nmeaTIME beiJingTime;    //北京时间 
  
    char str_buff[100];
  
  	/*使用不透明前景层*/
    LCD_SetLayer(LCD_FOREGROUND_LAYER);  
    LCD_SetTransparency(0xff);
    
    LCD_Clear(LCD_COLOR_BLACK);	/* 清屏，显示全黑 */

    /*设置字体颜色及字体的背景颜色(此处的背景不是指LCD的背景层！注意区分)*/
    LCD_SetColors(LCD_COLOR_RED,LCD_COLOR_BLACK);
  
  	LCD_DisplayStringLine(LINE(1),(uint8_t *)" Wildfire STM32-F429");
    LCD_DisplayStringLine(LINE(2),(uint8_t *)"  GPS module");

  

    /* 设置用于输出调试信息的函数 */
    nmea_property()->trace_func = &trace;
    nmea_property()->error_func = &error;
    nmea_property()->info_func = &gps_info;

    /* 初始化GPS数据结构 */
    nmea_zero_INFO(&info);
    nmea_parser_init(&parser);

    while(1)
    {
      if(GPS_HalfTransferEnd)     /* 接收到GPS_RBUFF_SIZE一半的数据 */
      {
        /* 进行nmea格式解码 */
        nmea_parse(&parser, (const char*)&gps_rbuff[0], HALF_GPS_RBUFF_SIZE, &info);
        
        GPS_HalfTransferEnd = 0;   //清空标志位
        new_parse = 1;             //设置解码消息标志 
      }
      else if(GPS_TransferEnd)    /* 接收到另一半数据 */
      {

        nmea_parse(&parser, (const char*)&gps_rbuff[HALF_GPS_RBUFF_SIZE], HALF_GPS_RBUFF_SIZE, &info);
       
        GPS_TransferEnd = 0;
        new_parse =1;
      }
      
      if(new_parse )                //有新的解码消息   
      {    
        /* 对解码后的时间进行转换，转换成北京时间 */
        GMTconvert(&info.utc,&beiJingTime,8,1);
        
        /* 输出解码得到的信息 */
				printf("\r\n时间%d-%02d-%02d,%d:%d:%d\r\n", beiJingTime.year+1900, beiJingTime.mon,beiJingTime.day,beiJingTime.hour,beiJingTime.min,beiJingTime.sec);
			
				//info.lat lon中的格式为[degree][min].[sec/60]，使用以下函数转换成[degree].[degree]格式
				deg_lat = nmea_ndeg2degree(info.lat);
				deg_lon = nmea_ndeg2degree(info.lon);
			
				printf("\r\n纬度：%f,经度%f\r\n",deg_lat,deg_lon);
        printf("\r\n海拔高度：%f 米 ", info.elv);
        printf("\r\n速度：%f km/h ", info.speed);
        printf("\r\n航向：%f 度", info.direction);
				
				printf("\r\n正在使用的GPS卫星：%d,可见GPS卫星：%d",info.satinfo.inuse,info.satinfo.inview);

				printf("\r\n正在使用的北斗卫星：%d,可见北斗卫星：%d",info.BDsatinfo.inuse,info.BDsatinfo.inview);
				printf("\r\nPDOP：%f,HDOP：%f，VDOP：%f",info.PDOP,info.HDOP,info.VDOP);
        
        
        /* 液晶输出 */
        
        /* 设置前景颜色（字体颜色）*/
        LCD_SetTextColor(LCD_COLOR_BLUE);
        
        LCD_DisplayStringLine(LINE(5),(uint8_t *)" GPS Info:");

        /* 设置前景颜色（字体颜色）*/
        LCD_SetTextColor(LCD_COLOR_WHITE);
        
        /* 显示时间日期 */
        sprintf(str_buff," Date:%04d/%02d/%02d Time:%02d:%02d:%02d", beiJingTime.year+1900, beiJingTime.mon,beiJingTime.day,beiJingTime.hour,beiJingTime.min,beiJingTime.sec);
        LCD_DisplayStringLine(LINE(6),(uint8_t *)str_buff);
      
        /* 纬度 经度*/
        sprintf(str_buff," latitude :%.6f ", deg_lat);
        LCD_DisplayStringLine(LINE(7),(uint8_t *)str_buff);
        
        sprintf(str_buff," longitude :%.6f",deg_lon);
        LCD_DisplayStringLine(LINE(8),(uint8_t *)str_buff);
        
        /* 正在使用的卫星 可见的卫星*/
        sprintf(str_buff," GPS  Satellite in use :%2d ", info.satinfo.inuse);
        LCD_DisplayStringLine(LINE(9),(uint8_t *)str_buff);    
        
       sprintf(str_buff," GPS Satellite in view :%2d", info.satinfo.inview);
        LCD_DisplayStringLine(LINE(10),(uint8_t *)str_buff);    

        /* 正在使用的卫星 可见的卫星*/
        sprintf(str_buff," BDS  Satellite in use :%2d ", info.BDsatinfo.inuse);
        LCD_DisplayStringLine(LINE(11),(uint8_t *)str_buff);    
        
       sprintf(str_buff," BDS Satellite in view :%2d", info.BDsatinfo.inview);
        LCD_DisplayStringLine(LINE(12),(uint8_t *)str_buff);    
        
        /* 海拔高度 */
        sprintf(str_buff," Altitude:%4.2f m", info.elv);
        LCD_DisplayStringLine(LINE(13),(uint8_t *)str_buff);
        
        /* 速度 */
        sprintf(str_buff," speed:%4.2f km/h", info.speed);
        LCD_DisplayStringLine(LINE(14),(uint8_t *)str_buff);
        
        /* 航向 */
        sprintf(str_buff," Track angle:%3.2f deg", info.direction);
        LCD_DisplayStringLine(LINE(15),(uint8_t *)str_buff);
        
        
        new_parse = 0;
      }
	
	}

    /* 释放GPS数据结构 */
    // nmea_parser_destroy(&parser);

    
    //  return 0;
}

#endif






/**************************************************end of file****************************************/

