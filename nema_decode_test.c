/**
  ******************************************************************************
  * @brief   ����NEMA�����
  ******************************************************************************
  */ 
  
#include "stm32f4xx.h"
#include "./usart/bsp_debug_usart.h"
#include "./lcd/bsp_lcd.h"
#include "./gps/gps_config.h"
#include "ff.h"
#include "nmea/nmea.h"

  

#ifdef __GPS_LOG_FILE             //��SD���ϵ�gpslog.txt�ļ����н��룻����Ҫ��sd���ϴ��gpslog.txt�ļ���

FATFS fs;
FIL log_file;
FRESULT res; 
UINT br, bw;            					/* File R/W count */

/**
  * @brief  nmea_decode_test ����GPS�ļ���Ϣ
  * @param  ��
  * @retval ��
  */
void nmea_decode_test(void)
{
		double deg_lat;//ת����[degree].[degree]��ʽ��γ��
		double deg_lon;//ת����[degree].[degree]��ʽ�ľ���

    nmeaINFO info;          //GPS�����õ�����Ϣ
    nmeaPARSER parser;      //����ʱʹ�õ����ݽṹ  
    
    nmeaTIME beiJingTime;    //����ʱ�� 

    char buff[2048];  
  
    char str_buff[100];
  
  	/*ʹ�ò�͸��ǰ����*/
    LCD_SetLayer(LCD_FOREGROUND_LAYER);  
    LCD_SetTransparency(0xff);
    
    LCD_Clear(LCD_COLOR_BLACK);	/* ��������ʾȫ�� */

    /*����������ɫ������ı�����ɫ(�˴��ı�������ָLCD�ı����㣡ע������)*/
    LCD_SetColors(LCD_COLOR_RED,LCD_COLOR_BLACK);
  
  	LCD_DisplayStringLine(LINE(1),(uint8_t *)" Wildfire STM32-F429");
    LCD_DisplayStringLine(LINE(2),(uint8_t *)"  GPS module");
    
  
    	/* ע���̷� */
    res = f_mount(&fs,"0:",1);	
  
    if(res != FR_OK)
    {
      printf("\r\n����SD�������ļ�ϵͳʧ�ܡ�(%d)��������������SD��\r\n",res);
      while(1);
    }

    /* �򿪼�¼��GPS��Ϣ���ļ� */
    res = f_open(&log_file,"0:gpslog.txt", FA_OPEN_EXISTING|FA_READ);

    if(!(res == FR_OK))
    {
        printf("\r\n��gpslog.txt�ļ�ʧ�ܣ�����SD���ĸ�Ŀ¼�Ƿ�����gpslog.txt�ļ�!\r\n");
        return ;      
    }
    

    /* �����������������Ϣ�ĺ��� */
    nmea_property()->trace_func = &trace;
    nmea_property()->error_func = &error;
    nmea_property()->info_func = &gps_info;

    /* ��ʼ��GPS���ݽṹ */
    nmea_zero_INFO(&info);
    nmea_parser_init(&parser);

    while(!f_eof(&log_file))
    {
      
        f_read(&log_file, &buff[0], 100, &br);

        /* ����nmea��ʽ���� */
        nmea_parse(&parser, &buff[0], br, &info);
      
        /* �Խ�����ʱ�����ת����ת���ɱ���ʱ�� */
        GMTconvert(&info.utc,&beiJingTime,8,1);
        
        /* �������õ�����Ϣ */
				printf("\r\nʱ��%d-%02d-%02d,%d:%d:%d\r\n", beiJingTime.year+1900, beiJingTime.mon,beiJingTime.day,beiJingTime.hour,beiJingTime.min,beiJingTime.sec);
				
				//info.lat lon�еĸ�ʽΪ[degree][min].[sec/60]��ʹ�����º���ת����[degree].[degree]��ʽ
				deg_lat = nmea_ndeg2degree(info.lat);
				deg_lon = nmea_ndeg2degree(info.lon);
			
				printf("\r\nγ�ȣ�%f,����%f\r\n",deg_lat,deg_lon);
        printf("\r\n���θ߶ȣ�%f �� ", info.elv);
        printf("\r\n�ٶȣ�%f km/h ", info.speed);
        printf("\r\n����%f ��", info.direction);
				
				printf("\r\n����ʹ�õ�GPS���ǣ�%d,�ɼ�GPS���ǣ�%d",info.satinfo.inuse,info.satinfo.inview);

				printf("\r\n����ʹ�õı������ǣ�%d,�ɼ��������ǣ�%d",info.BDsatinfo.inuse,info.BDsatinfo.inview);
				printf("\r\nPDOP��%f,HDOP��%f��VDOP��%f",info.PDOP,info.HDOP,info.VDOP);
           
        /* Һ����� */
        
        /* ����ǰ����ɫ��������ɫ��*/
        LCD_SetTextColor(LCD_COLOR_BLUE);
        
        LCD_DisplayStringLine(LINE(5),(uint8_t *)" GPS Info:");

        /* ����ǰ����ɫ��������ɫ��*/
        LCD_SetTextColor(LCD_COLOR_WHITE);
        
        /* ��ʾʱ������ */
        sprintf(str_buff," Date:%04d/%02d/%02d Time:%02d:%02d:%02d", beiJingTime.year+1900, beiJingTime.mon,beiJingTime.day,beiJingTime.hour,beiJingTime.min,beiJingTime.sec);
        LCD_DisplayStringLine(LINE(6),(uint8_t *)str_buff);
      
        /* γ�� ����*/
        sprintf(str_buff," latitude :%.6f ", deg_lat);
        LCD_DisplayStringLine(LINE(7),(uint8_t *)str_buff);
        
        sprintf(str_buff," longitude :%.6f",deg_lon);
        LCD_DisplayStringLine(LINE(8),(uint8_t *)str_buff);
        
        /* ����ʹ�õ����� �ɼ�������*/
        sprintf(str_buff," GPS  Satellite in use :%2d ", info.satinfo.inuse);
        LCD_DisplayStringLine(LINE(9),(uint8_t *)str_buff);    
        
       sprintf(str_buff," GPS Satellite in view :%2d", info.satinfo.inview);
        LCD_DisplayStringLine(LINE(10),(uint8_t *)str_buff);    

        /* ����ʹ�õ����� �ɼ�������*/
        sprintf(str_buff," BDS  Satellite in use :%2d ", info.BDsatinfo.inuse);
        LCD_DisplayStringLine(LINE(11),(uint8_t *)str_buff);    
        
       sprintf(str_buff," BDS Satellite in view :%2d", info.BDsatinfo.inview);
        LCD_DisplayStringLine(LINE(12),(uint8_t *)str_buff);    
        
        /* ���θ߶� */
        sprintf(str_buff," Altitude:%4.2f m", info.elv);
        LCD_DisplayStringLine(LINE(13),(uint8_t *)str_buff);
        
        /* �ٶ� */
        sprintf(str_buff," speed:%4.2f km/h", info.speed);
        LCD_DisplayStringLine(LINE(14),(uint8_t *)str_buff);
        
        /* ���� */
        sprintf(str_buff," Track angle:%3.2f deg", info.direction);
        LCD_DisplayStringLine(LINE(15),(uint8_t *)str_buff);
        
        
	
	}

    f_lseek(&log_file, f_size(&log_file));

    /* �ͷ�GPS���ݽṹ */
    nmea_parser_destroy(&parser);
  
    /* �ر��ļ� */
    f_close(&log_file);
    
   
}
#else       //��GPSģ�鴫�ص���Ϣ���н���

/**
  * @brief  nmea_decode_test ����GPSģ����Ϣ
  * @param  ��
  * @retval ��
  */
int nmea_decode_test(void)
{
		double deg_lat;//ת����[degree].[degree]��ʽ��γ��
		double deg_lon;//ת����[degree].[degree]��ʽ�ľ���

    nmeaINFO info;          //GPS�����õ�����Ϣ
    nmeaPARSER parser;      //����ʱʹ�õ����ݽṹ  
    uint8_t new_parse=0;    //�Ƿ����µĽ������ݱ�־
  
    nmeaTIME beiJingTime;    //����ʱ�� 
  
    char str_buff[100];
  
  	/*ʹ�ò�͸��ǰ����*/
    LCD_SetLayer(LCD_FOREGROUND_LAYER);  
    LCD_SetTransparency(0xff);
    
    LCD_Clear(LCD_COLOR_BLACK);	/* ��������ʾȫ�� */

    /*����������ɫ������ı�����ɫ(�˴��ı�������ָLCD�ı����㣡ע������)*/
    LCD_SetColors(LCD_COLOR_RED,LCD_COLOR_BLACK);
  
  	LCD_DisplayStringLine(LINE(1),(uint8_t *)" Wildfire STM32-F429");
    LCD_DisplayStringLine(LINE(2),(uint8_t *)"  GPS module");

  

    /* �����������������Ϣ�ĺ��� */
    nmea_property()->trace_func = &trace;
    nmea_property()->error_func = &error;
    nmea_property()->info_func = &gps_info;

    /* ��ʼ��GPS���ݽṹ */
    nmea_zero_INFO(&info);
    nmea_parser_init(&parser);

    while(1)
    {
      if(GPS_HalfTransferEnd)     /* ���յ�GPS_RBUFF_SIZEһ������� */
      {
        /* ����nmea��ʽ���� */
        nmea_parse(&parser, (const char*)&gps_rbuff[0], HALF_GPS_RBUFF_SIZE, &info);
        
        GPS_HalfTransferEnd = 0;   //��ձ�־λ
        new_parse = 1;             //���ý�����Ϣ��־ 
      }
      else if(GPS_TransferEnd)    /* ���յ���һ������ */
      {

        nmea_parse(&parser, (const char*)&gps_rbuff[HALF_GPS_RBUFF_SIZE], HALF_GPS_RBUFF_SIZE, &info);
       
        GPS_TransferEnd = 0;
        new_parse =1;
      }
      
      if(new_parse )                //���µĽ�����Ϣ   
      {    
        /* �Խ�����ʱ�����ת����ת���ɱ���ʱ�� */
        GMTconvert(&info.utc,&beiJingTime,8,1);
        
        /* �������õ�����Ϣ */
				printf("\r\nʱ��%d-%02d-%02d,%d:%d:%d\r\n", beiJingTime.year+1900, beiJingTime.mon,beiJingTime.day,beiJingTime.hour,beiJingTime.min,beiJingTime.sec);
			
				//info.lat lon�еĸ�ʽΪ[degree][min].[sec/60]��ʹ�����º���ת����[degree].[degree]��ʽ
				deg_lat = nmea_ndeg2degree(info.lat);
				deg_lon = nmea_ndeg2degree(info.lon);
			
				printf("\r\nγ�ȣ�%f,����%f\r\n",deg_lat,deg_lon);
        printf("\r\n���θ߶ȣ�%f �� ", info.elv);
        printf("\r\n�ٶȣ�%f km/h ", info.speed);
        printf("\r\n����%f ��", info.direction);
				
				printf("\r\n����ʹ�õ�GPS���ǣ�%d,�ɼ�GPS���ǣ�%d",info.satinfo.inuse,info.satinfo.inview);

				printf("\r\n����ʹ�õı������ǣ�%d,�ɼ��������ǣ�%d",info.BDsatinfo.inuse,info.BDsatinfo.inview);
				printf("\r\nPDOP��%f,HDOP��%f��VDOP��%f",info.PDOP,info.HDOP,info.VDOP);
        
        
        /* Һ����� */
        
        /* ����ǰ����ɫ��������ɫ��*/
        LCD_SetTextColor(LCD_COLOR_BLUE);
        
        LCD_DisplayStringLine(LINE(5),(uint8_t *)" GPS Info:");

        /* ����ǰ����ɫ��������ɫ��*/
        LCD_SetTextColor(LCD_COLOR_WHITE);
        
        /* ��ʾʱ������ */
        sprintf(str_buff," Date:%04d/%02d/%02d Time:%02d:%02d:%02d", beiJingTime.year+1900, beiJingTime.mon,beiJingTime.day,beiJingTime.hour,beiJingTime.min,beiJingTime.sec);
        LCD_DisplayStringLine(LINE(6),(uint8_t *)str_buff);
      
        /* γ�� ����*/
        sprintf(str_buff," latitude :%.6f ", deg_lat);
        LCD_DisplayStringLine(LINE(7),(uint8_t *)str_buff);
        
        sprintf(str_buff," longitude :%.6f",deg_lon);
        LCD_DisplayStringLine(LINE(8),(uint8_t *)str_buff);
        
        /* ����ʹ�õ����� �ɼ�������*/
        sprintf(str_buff," GPS  Satellite in use :%2d ", info.satinfo.inuse);
        LCD_DisplayStringLine(LINE(9),(uint8_t *)str_buff);    
        
       sprintf(str_buff," GPS Satellite in view :%2d", info.satinfo.inview);
        LCD_DisplayStringLine(LINE(10),(uint8_t *)str_buff);    

        /* ����ʹ�õ����� �ɼ�������*/
        sprintf(str_buff," BDS  Satellite in use :%2d ", info.BDsatinfo.inuse);
        LCD_DisplayStringLine(LINE(11),(uint8_t *)str_buff);    
        
       sprintf(str_buff," BDS Satellite in view :%2d", info.BDsatinfo.inview);
        LCD_DisplayStringLine(LINE(12),(uint8_t *)str_buff);    
        
        /* ���θ߶� */
        sprintf(str_buff," Altitude:%4.2f m", info.elv);
        LCD_DisplayStringLine(LINE(13),(uint8_t *)str_buff);
        
        /* �ٶ� */
        sprintf(str_buff," speed:%4.2f km/h", info.speed);
        LCD_DisplayStringLine(LINE(14),(uint8_t *)str_buff);
        
        /* ���� */
        sprintf(str_buff," Track angle:%3.2f deg", info.direction);
        LCD_DisplayStringLine(LINE(15),(uint8_t *)str_buff);
        
        
        new_parse = 0;
      }
	
	}

    /* �ͷ�GPS���ݽṹ */
    // nmea_parser_destroy(&parser);

    
    //  return 0;
}

#endif






/**************************************************end of file****************************************/
