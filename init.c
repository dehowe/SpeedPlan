#include "init.h"

/*************************************************************************
* 功能描述: 填充车站数据
* 输入参数: 无
* 输出参数: 无
* 返回值:   无
*************************************************************************/
void FillStationData(char *data,UINT16 row_index,UINT16 column_index)
{
    switch (column_index)
    {
        case 0:
            g_static_data_csv.station_csv.length+=1;
            memcpy(g_static_data_csv.station_csv.station_name[row_index], data, strlen(data));
            break;
        case 1:
            g_static_data_csv.station_csv.station_id[row_index] = (UINT16)atoi(data);
            break;
        case 2:
            memcpy(g_static_data_csv.station_csv.property[row_index], data, strlen(data));
            break;
        case 3:
            memcpy(g_static_data_csv.station_csv.property_display[row_index], data, strlen(data));
            break;
        case 4:
            g_static_data_csv.station_csv.schedule_time[row_index] = (FLOAT32)atof(data);
            break;
        case 5:
            g_static_data_csv.station_csv.dwell_time[row_index] = (FLOAT32)atof(data);
            break;
        case 6:
            g_static_data_csv.station_csv.traction_energy[row_index] = (FLOAT32)atof(data);
            break;
        case 7:
            g_static_data_csv.station_csv.regenerative_energy[row_index] = (FLOAT32)atof(data);
            break;
        case 8:
            g_static_data_csv.station_csv.operation_energy[row_index] = (FLOAT32)atof(data);
            break;
        case 9:
            g_static_data_csv.station_csv.begin_distance[row_index] = (FLOAT32)atof(data);
            break;
        case 10:
            g_static_data_csv.station_csv.end_distance[row_index] = (FLOAT32)atof(data);
            break;
        default:
            break;
    }
}
/*************************************************************************
* 功能描述: 填充限速数据
* 输入参数: 无
* 输出参数: 无
* 返回值:   无
*************************************************************************/
void FillSpeedLimitData(char *data,UINT16 row_index,UINT16 column_index)
{
    switch (column_index)
    {
        case 0:
            g_static_data_csv.speed_limit_csv.length+=1;
            g_static_data_csv.speed_limit_csv.distance[row_index] = (UINT32)atoi(data);
            break;
        case 1:
            g_static_data_csv.speed_limit_csv.speed_limit[row_index] = (UINT16)atoi(data);
            break;
        default:
            break;
    }
}
/*************************************************************************
* 功能描述: 填充坡度数据
* 输入参数: 无
* 输出参数: 无
* 返回值:   无
*************************************************************************/
void FillGradientData(char *data,UINT16 row_index,UINT16 column_index)
{
    switch (column_index)
    {
        case 0:
            g_static_data_csv.gradient_csv.length+=1;
            g_static_data_csv.gradient_csv.distance[row_index] = (UINT32)atoi(data);
            break;
        case 1:
            g_static_data_csv.gradient_csv.gradient[row_index] = (FLOAT32)atof(data);
            break;
        case 2:
            g_static_data_csv.gradient_csv.vertical_curve_radius[row_index] = (UINT32)atoi(data);
            break;
        default:
            break;
    }
}

/*************************************************************************
* 功能描述: 填充隧道数据
* 输入参数: 无
* 输出参数: 无
* 返回值:   无
*************************************************************************/
void FillTunnelData(char *data,UINT16 row_index,UINT16 column_index)
{
    switch (column_index)
    {
        case 0:
            g_static_data_csv.tunnel_csv.length+=1;
            g_static_data_csv.tunnel_csv.begin_distance[row_index] = (UINT32)atoi(data);
            break;
        case 1:
            g_static_data_csv.tunnel_csv.end_distance[row_index] = (UINT32)atoi(data);
            break;
        case 2:
            g_static_data_csv.tunnel_csv.tunnel_param[row_index] = (UINT16)atoi(data);
            break;
        default:
            break;
    }
}

/*************************************************************************
* 功能描述: 填充曲线半径数据
* 输入参数: 无
* 输出参数: 无
* 返回值:   无
*************************************************************************/
void FillCurveRadiusData(char *data,UINT16 row_index,UINT16 column_index)
{
    switch (column_index)
    {
        case 0:
            g_static_data_csv.curve_radius_csv.length+=1;
            g_static_data_csv.curve_radius_csv.distance[row_index] = (UINT32)atoi(data);
            break;
        case 1:
            g_static_data_csv.curve_radius_csv.curve_radius[row_index] = (UINT16)atoi(data);
            break;
        default:
            break;
    }
}

/*************************************************************************
* 功能描述: 填充列车基本参数数据
* 输入参数: 无
* 输出参数: 无
* 返回值:   无
*************************************************************************/
void FillBasicParamData(char *data,UINT16 row_index,UINT16 column_index)
{
    switch (column_index)
    {
        case 0:
            g_static_data_csv.basic_param_csv.length+=1;
            g_static_data_csv.basic_param_csv.number[row_index] = (UINT8)atoi(data);
            break;
        case 1:
            memcpy(g_static_data_csv.basic_param_csv.property[row_index], data, strlen(data));
            break;
        case 2:
            memcpy(g_static_data_csv.basic_param_csv.property_display[row_index], data, strlen(data));
            break;
        case 3:
            memcpy(g_static_data_csv.basic_param_csv.type[row_index], data, strlen(data));
            break;
        case 4:
            g_static_data_csv.basic_param_csv.train_length[row_index] = (UINT16)atoi(data);
            break;
        case 5:
            g_static_data_csv.basic_param_csv.max_speed[row_index] = (UINT16)atoi(data);
            break;
        case 6:
            memcpy(g_static_data_csv.basic_param_csv.capacity[row_index], data, strlen(data));
            break;
        case 7:
            g_static_data_csv.basic_param_csv.train_weight[row_index] = (FLOAT32)atof(data);
            break;
        case 8:
            g_static_data_csv.basic_param_csv.basic_a[row_index] = (FLOAT32)atof(data);
            break;
        case 9:
            g_static_data_csv.basic_param_csv.basic_b[row_index] = (FLOAT32)atof(data);
            break;
        case 10:
            g_static_data_csv.basic_param_csv.basic_c[row_index] = (FLOAT32)atof(data);
            break;
        case 11:
            g_static_data_csv.basic_param_csv.transformer[row_index] = (FLOAT32)atof(data);
            break;
        case 12:
            g_static_data_csv.basic_param_csv.converter[row_index] = (FLOAT32)atof(data);
            break;
        case 13:
            g_static_data_csv.basic_param_csv.gearboxes[row_index] = (FLOAT32)atof(data);
            break;
        default:
            break;
    }
}

/*************************************************************************
* 功能描述: 填充动力学数据
* 输入参数: 无
* 输出参数: 无
* 返回值:   无
*************************************************************************/
void FillDynamicsData(char *data,UINT16 row_index,UINT16 column_index)
{
    switch (column_index)
    {
        case 0:
            g_static_data_csv.dynamics_csv.length+=1;
            g_static_data_csv.dynamics_csv.speed[row_index] = (UINT16)atoi(data);
            break;
        case 1:
            g_static_data_csv.dynamics_csv.traction_aw0[row_index] = (FLOAT32)atof(data);
            break;
        case 2:
            g_static_data_csv.dynamics_csv.traction_motor_aw0[row_index] = (FLOAT32)atof(data);
            break;
        case 3:
            g_static_data_csv.dynamics_csv.brake_aw0[row_index] = (FLOAT32)atof(data);
            break;
        case 4:
            g_static_data_csv.dynamics_csv.brake_motor_aw0[row_index] = (FLOAT32)atof(data);
            break;
        case 5:
            g_static_data_csv.dynamics_csv.traction_aw1[row_index] = (FLOAT32)atof(data);
            break;
        case 6:
            g_static_data_csv.dynamics_csv.traction_motor_aw1[row_index] = (FLOAT32)atof(data);
            break;
        case 7:
            g_static_data_csv.dynamics_csv.brake_aw1[row_index] = (FLOAT32)atof(data);
            break;
        case 8:
            g_static_data_csv.dynamics_csv.brake_motor_aw1[row_index] = (FLOAT32)atof(data);
            break;
        case 9:
            g_static_data_csv.dynamics_csv.traction_aw2[row_index] = (FLOAT32)atof(data);
            break;
        case 10:
            g_static_data_csv.dynamics_csv.traction_motor_aw2[row_index] = (FLOAT32)atof(data);
            break;
        case 11:
            g_static_data_csv.dynamics_csv.brake_aw2[row_index] = (FLOAT32)atof(data);
            break;
        case 12:
            g_static_data_csv.dynamics_csv.brake_motor_aw2[row_index] = (FLOAT32)atof(data);
            break;
        case 13:
            g_static_data_csv.dynamics_csv.traction_aw3[row_index] = (FLOAT32)atof(data);
            break;
        case 14:
            g_static_data_csv.dynamics_csv.traction_motor_aw3[row_index] = (FLOAT32)atof(data);
            break;
        case 15:
            g_static_data_csv.dynamics_csv.brake_aw3[row_index] = (FLOAT32)atof(data);
            break;
        case 16:
            g_static_data_csv.dynamics_csv.brake_motor_aw3[row_index] = (FLOAT32)atof(data);
            break;
        case 17:
            g_static_data_csv.dynamics_csv.traction_aw4[row_index] = (FLOAT32)atof(data);
            break;
        case 18:
            g_static_data_csv.dynamics_csv.traction_motor_aw4[row_index] = (FLOAT32)atof(data);
            break;
        case 19:
            g_static_data_csv.dynamics_csv.brake_aw4[row_index] = (FLOAT32)atof(data);
            break;
        case 20:
            g_static_data_csv.dynamics_csv.brake_motor_aw4[row_index] = (FLOAT32)atof(data);
            break;
        case 21:
            g_static_data_csv.dynamics_csv.traction_aw5[row_index] = (FLOAT32)atof(data);
            break;
        case 22:
            g_static_data_csv.dynamics_csv.traction_motor_aw5[row_index] = (FLOAT32)atof(data);
            break;
        case 23:
            g_static_data_csv.dynamics_csv.brake_aw5[row_index] = (FLOAT32)atof(data);
            break;
        case 24:
            g_static_data_csv.dynamics_csv.brake_motor_aw5[row_index] = (FLOAT32)atof(data);
            break;
        default:
            break;
    }
}

/*************************************************************************
* 功能描述: 填充曲线优化离线数据
* 输入参数: 无
* 输出参数: 无
* 返回值:   无
*************************************************************************/
void FillOptimizeData(char *data,UINT16 row_index,UINT16 column_index)
{
    switch (column_index)
    {
        case 0:
            g_static_data_csv.optimize_csv.length+=1;
            g_static_data_csv.optimize_csv.distance[row_index] = (UINT32)atoi(data);
            break;
        case 1:
            g_static_data_csv.optimize_csv.speed_down_aw0[row_index] = (FLOAT32)atof(data);
            break;
        case 2:
            g_static_data_csv.optimize_csv.level_flag_down_aw0[row_index] = (UINT8)atoi(data);
            break;
        case 3:
            g_static_data_csv.optimize_csv.level_output_down_aw0[row_index] = (UINT16)atoi(data);
            break;
        case 4:
            g_static_data_csv.optimize_csv.speed_down_aw1[row_index] = (FLOAT32)atof(data);
            break;
        case 5:
            g_static_data_csv.optimize_csv.level_flag_down_aw1[row_index] = (UINT8)atoi(data);
            break;
        case 6:
            g_static_data_csv.optimize_csv.level_output_down_aw1[row_index] = (UINT16)atoi(data);
            break;
        case 7:
            g_static_data_csv.optimize_csv.speed_down_aw2[row_index] = (FLOAT32)atof(data);
            break;
        case 8:
            g_static_data_csv.optimize_csv.level_flag_down_aw2[row_index] = (UINT8)atoi(data);
            break;
        case 9:
            g_static_data_csv.optimize_csv.level_output_down_aw2[row_index] = (UINT16)atoi(data);
            break;
        case 10:
            g_static_data_csv.optimize_csv.speed_down_aw3[row_index] = (FLOAT32)atof(data);
            break;
        case 11:
            g_static_data_csv.optimize_csv.level_flag_down_aw3[row_index] = (UINT8)atoi(data);
            break;
        case 12:
            g_static_data_csv.optimize_csv.level_output_down_aw3[row_index] = (UINT16)atoi(data);
            break;
        case 13:
            g_static_data_csv.optimize_csv.speed_up_aw0[row_index] = (FLOAT32)atof(data);
            break;
        case 14:
            g_static_data_csv.optimize_csv.level_flag_up_aw0[row_index] = (UINT8)atoi(data);
            break;
        case 15:
            g_static_data_csv.optimize_csv.level_output_up_aw0[row_index] = (UINT16)atoi(data);
            break;
        case 16:
            g_static_data_csv.optimize_csv.speed_up_aw1[row_index] = (FLOAT32)atof(data);
            break;
        case 17:
            g_static_data_csv.optimize_csv.level_flag_up_aw1[row_index] = (UINT8)atoi(data);
            break;
        case 18:
            g_static_data_csv.optimize_csv.level_output_up_aw1[row_index] = (UINT16)atoi(data);
            break;
        case 19:
            g_static_data_csv.optimize_csv.speed_up_aw2[row_index] = (FLOAT32)atof(data);
            break;
        case 20:
            g_static_data_csv.optimize_csv.level_flag_up_aw2[row_index] = (UINT8)atoi(data);
            break;
        case 21:
            g_static_data_csv.optimize_csv.level_output_up_aw2[row_index] = (UINT16)atoi(data);
            break;
        case 22:
            g_static_data_csv.optimize_csv.speed_up_aw3[row_index] = (FLOAT32)atof(data);
            break;
        case 23:
            g_static_data_csv.optimize_csv.level_flag_up_aw3[row_index] = (UINT8)atoi(data);
            break;
        case 24:
            g_static_data_csv.optimize_csv.level_output_up_aw3[row_index] = (UINT16)atoi(data);
            break;
        default:
            break;
    }
}

/*************************************************************************
* 功能描述: 填充CSV数据
* 输入参数: 无
* 输出参数: 无
* 返回值:   无
*************************************************************************/
int FillCsvData(char *data,UINT16 row_index,UINT16 column_index,UINT16 label_flag)
{
    switch (label_flag)
    {
        case 1:
            FillStationData(data,row_index,column_index);
            break;
        case 2:
            FillSpeedLimitData(data,row_index,column_index);
            break;
        case 3:
            FillGradientData(data,row_index,column_index);
            break;
        case 4:
            FillTunnelData(data,row_index,column_index);
            break;
        case 5:
            FillCurveRadiusData(data,row_index,column_index);
            break;
        case 6:
            FillBasicParamData(data,row_index,column_index);
            break;
        case 7:
            FillDynamicsData(data,row_index,column_index);
            break;
        case 8:
            FillOptimizeData(data,row_index,column_index);
            break;
        default:
            break;
    }

}

/*************************************************************************
* 功能描述: CSV
* 输入参数: 无
* 输出参数: 无
* 返回值:   无
*************************************************************************/
int ReadCsvConfig(char *filename)
{
    FILE *fp=NULL;
    char path[512]={0x0};
    int label_flag_temp = 0;
    int rows_temp = 0;
    int columns_temp = 0;
    getcwd(path,sizeof(path));
    strcat(path,filename);
    fp= fopen(path,"r");
    if(NULL==fp)
    {
        return 0;
    }
    char table_config[3][3][10];
    for (int i = 0; i <2; i++)
    {
        for(int j=0;j<3;j++)
        {
            fscanf(fp,"%[^,\n]",table_config[i][j]);
            fgetc(fp);
        }
    }
    label_flag_temp=atoi(table_config[1][0]);
    rows_temp=atoi(table_config[1][1]);
    columns_temp=atoi(table_config[1][2]);
    for(int i=0;i<columns_temp;i++)
    {
        char data_temp[10];
        fscanf(fp,"%[^,\n]",data_temp);
        fgetc(fp);
    }

    for (int i = 0; i <rows_temp; i++)
    {
        for(int j=0;j<columns_temp;j++)
        {
            char data[50];
            fscanf(fp,"%[^,\n]",data);
            fgetc(fp);
            //int debug=atoi(data);
            FillCsvData(data,i,j,label_flag_temp);
        }
    }

    fclose(fp);
    fp=NULL;
    return 1;
}

/*************************************************************************
 * 功能描述: 读取CSV静态数据文件
 * 输入参数: 无
 * 输出参数: 无
 * 返回值:   UINT8   1:读取成功 0：读取失败
 *************************************************************************/
UINT8 StaticDataRead()
{
    UINT8 result=0;
    /*车站静态数据初始化*/
    result = ReadCsvConfig("/LineData/LineDataStation.csv");
    if (result==0)
    {
        return result;
    }
    result = ReadCsvConfig("/LineData/LineDataSpeedLimit.csv");
    if (result==0)
    {
        return result;
    }
    result = ReadCsvConfig("/LineData/LineDataGradient.csv");
    if (result==0)
    {
        return result;
    }
    result = ReadCsvConfig("/LineData/LineDataCurveRadius.csv");
    if (result==0)
    {
        return result;
    }
    result = ReadCsvConfig("/TrainData/TrainDataBasicParam.csv");
    if (result==0)
    {
        return result;
    }
    result = ReadCsvConfig("/TrainData/TrainDataDynamics.csv");
    if (result==0)
    {
        return result;
    }
    result = ReadCsvConfig("/TrainData/OptimizeOfflineData.csv");
    if (result==0)
    {
        return result;
    }
    return 1;
}

/*************************************************************************
 * 功能描述: 列车静态数据初始化
 * 输入参数: 无
 * 输出参数: 无
 * 返回值:   UINT8   1:成功 0：失败
 *************************************************************************/
UINT8 TrainStaticDataInit()
{
    memset(&g_train_param,0,sizeof(g_train_param));//清空
    for (int i = 0; i < g_static_data_csv.basic_param_csv.length; i++)
    {
        char aw_str[50];
        memcpy(aw_str,g_static_data_csv.basic_param_csv.capacity[i],sizeof(g_static_data_csv.basic_param_csv.capacity[i]));
        //AW0
        if(strcmp(aw_str,"AW0")==0)
        {
            /*载荷*/
            g_train_param.aw[0]=(UINT16)g_static_data_csv.basic_param_csv.train_weight[i];
            /*戴维斯基本阻力系数，分别对应AW0、AW1、AW2、AW3载荷下a、b、c系数值 f=a+b*v+c*v*v 速度单位km/h 力单位kN*/
            g_train_param.a[0]=g_static_data_csv.basic_param_csv.basic_a[i];
            g_train_param.b[0]=g_static_data_csv.basic_param_csv.basic_b[i];
            g_train_param.c[0]=g_static_data_csv.basic_param_csv.basic_c[i];
            /*车长*/
            g_train_param.train_length=g_static_data_csv.basic_param_csv.train_length[i]*100;//车长cm
        }
        //AW1
        if(strcmp(aw_str,"AW1")==0)
        {
            g_train_param.aw[1]=(UINT16)g_static_data_csv.basic_param_csv.train_weight[i];
            g_train_param.a[1]=g_static_data_csv.basic_param_csv.basic_a[i];
            g_train_param.b[1]=g_static_data_csv.basic_param_csv.basic_b[i];
            g_train_param.c[1]=g_static_data_csv.basic_param_csv.basic_c[i];
        }
        //AW2
        if(strcmp(aw_str,"AW2")==0)
        {
            g_train_param.aw[2]=(UINT16)g_static_data_csv.basic_param_csv.train_weight[i];
            g_train_param.a[2]=g_static_data_csv.basic_param_csv.basic_a[i];
            g_train_param.b[2]=g_static_data_csv.basic_param_csv.basic_b[i];
            g_train_param.c[2]=g_static_data_csv.basic_param_csv.basic_c[i];
        }
        //AW3
        if(strcmp(aw_str,"AW3")==0)
        {
            g_train_param.aw[3]=(UINT16)g_static_data_csv.basic_param_csv.train_weight[i];
            g_train_param.a[3]=g_static_data_csv.basic_param_csv.basic_a[i];
            g_train_param.b[3]=g_static_data_csv.basic_param_csv.basic_b[i];
            g_train_param.c[3]=g_static_data_csv.basic_param_csv.basic_c[i];
        }

    }

    /*牵引制动特性*/
    memcpy(g_train_param.traction_v, g_static_data_csv.dynamics_csv.speed, g_static_data_csv.dynamics_csv.length * sizeof(UINT16));//速度
    UINT16 acc_aw_temp[MAX_DYNAMICS_CSV_NUM];//加速度临时变量 cm/s^2
    /*牵引特性曲线，不同速度下（km/h）、AW0、AW2、AW3载荷下，列车能够输出的最大牵引加速度cm/s^2 */
    g_train_param.traction_num = g_static_data_csv.dynamics_csv.length;
    if(g_train_param.aw[0]!=0)
    {
        for (int j = 0; j < g_train_param.traction_num; j++)
        {
            acc_aw_temp[j]=(UINT16)(100.0*g_static_data_csv.dynamics_csv.traction_aw0[j]/g_train_param.aw[0]);
        }
        memcpy(g_train_param.traction_a0, acc_aw_temp, g_train_param.traction_num * sizeof(UINT16));
    }
    if(g_train_param.aw[1]!=0)
    {
        for (int j = 0; j < g_train_param.traction_num; j++)
        {
            acc_aw_temp[j]=(UINT16)(100.0*g_static_data_csv.dynamics_csv.traction_aw1[j]/g_train_param.aw[1]);
        }
        memcpy(g_train_param.traction_a1, acc_aw_temp, g_train_param.traction_num * sizeof(UINT16));
    }
    if(g_train_param.aw[2]!=0)
    {
        for (int j = 0; j < g_train_param.traction_num; j++)
        {
            acc_aw_temp[j]=(UINT16)(100.0*g_static_data_csv.dynamics_csv.traction_aw2[j]/g_train_param.aw[2]);
        }
        memcpy(g_train_param.traction_a2, acc_aw_temp, g_train_param.traction_num * sizeof(UINT16));
    }
    if(g_train_param.aw[3]!=0)
    {
        for (int j = 0; j < g_train_param.traction_num; j++)
        {
            acc_aw_temp[j]=(UINT16)(100.0*g_static_data_csv.dynamics_csv.traction_aw3[j]/g_train_param.aw[3]);
        }
        memcpy(g_train_param.traction_a3, acc_aw_temp, g_train_param.traction_num * sizeof(UINT16));
    }

    /*制动特性曲线,不同速度下（km/h）、AW0、AW2、AW3载荷下，列车能够输出的最大制动加速度cm/s^2 */
    g_train_param.braking_num = g_static_data_csv.dynamics_csv.length;
    if(g_train_param.aw[0]!=0)
    {
        for (int j = 0; j < g_train_param.braking_num; j++)
        {
            acc_aw_temp[j]=(UINT16)(100.0*g_static_data_csv.dynamics_csv.brake_aw0[j]/g_train_param.aw[0]);
        }
        memcpy(g_train_param.braking_a0, acc_aw_temp, g_train_param.braking_num * sizeof(UINT16));
    }
    if(g_train_param.aw[1]!=0)
    {
        for (int j = 0; j < g_train_param.braking_num; j++)
        {
            acc_aw_temp[j]=(UINT16)(100.0*g_static_data_csv.dynamics_csv.brake_aw1[j]/g_train_param.aw[1]);
        }
        memcpy(g_train_param.braking_a1, acc_aw_temp, g_train_param.braking_num * sizeof(UINT16));
    }
    if(g_train_param.aw[2]!=0)
    {
        for (int j = 0; j < g_train_param.braking_num; j++)
        {
            acc_aw_temp[j]=(UINT16)(100.0*g_static_data_csv.dynamics_csv.brake_aw2[j]/g_train_param.aw[2]);
        }
        memcpy(g_train_param.braking_a2, acc_aw_temp, g_train_param.braking_num * sizeof(UINT16));
    }
    if(g_train_param.aw[3]!=0)
    {
        for (int j = 0; j < g_train_param.braking_num; j++)
        {
            acc_aw_temp[j]=(UINT16)(100.0*g_static_data_csv.dynamics_csv.brake_aw3[j]/g_train_param.aw[3]);
        }
        memcpy(g_train_param.braking_a3, acc_aw_temp, g_train_param.braking_num * sizeof(UINT16));
    }
    return 1;
}

/*************************************************************************
 * 功能描述: 根据车头公里标计算车尾公里标
 * 输入参数: UINT32 train_head_loc   列车车头公里标
 *          UINT16 train_length    列车车长 m
 * 输出参数: 无
 * 返回值:   UINT32   车尾公里标
 *************************************************************************/
UINT32 GetTrainTailLocation(UINT32 train_head_loc,UINT16 train_length)
{
    UINT32 train_tail_loc=0;
    if(train_head_loc<=train_length)
    {
        return 0;
    }
    else
    {
        return train_head_loc-train_length;
    }
}
/*************************************************************************
 * 功能描述: 根据车头车尾位置计算等效坡度值
 * 输入参数: 无
 * 输出参数: 无
 * 返回值:   FLOAT32   等效坡度 千分之一
 *************************************************************************/
FLOAT32 GetEquivalentGradient(UINT32 train_head_loc,UINT32 train_tail_loc)
{
    FLOAT32 train_head_gradient;//列车头部坡度
    UINT16 train_head_offset;//列车头部相对坡度切换点偏移
    FLOAT32 train_tail_gradient;//列车尾部坡度
    UINT16 train_tail_offset;//列车尾部相对坡度切换点偏移
    FLOAT32 equivalent_gradient_temp;
    for (int i=0;i<g_static_data_csv.gradient_csv.length-1;i++)
    {
        //如果列车处于坡度切换点
        if(g_static_data_csv.gradient_csv.distance[i]>train_tail_loc&&g_static_data_csv.gradient_csv.distance[i]<train_head_loc)
        {
            train_head_gradient=g_static_data_csv.gradient_csv.gradient[i];
            train_tail_gradient=g_static_data_csv.gradient_csv.gradient[i-1];
            train_head_offset=train_head_loc-g_static_data_csv.gradient_csv.distance[i];
            train_tail_offset=g_static_data_csv.gradient_csv.distance[i]-train_tail_loc;
            equivalent_gradient_temp=(train_head_gradient*(FLOAT32)train_head_offset+train_tail_gradient*(FLOAT32)train_tail_offset)/((FLOAT32)g_train_param.train_length/100);
            return equivalent_gradient_temp;
        }
        else if(train_tail_loc>=g_static_data_csv.gradient_csv.distance[i]&&train_head_loc<=g_static_data_csv.gradient_csv.distance[i+1])
        {
            equivalent_gradient_temp=g_static_data_csv.gradient_csv.gradient[i];
            return equivalent_gradient_temp;
        }
        else
        {
            continue;
        }

    }
    return 0;

}


/*************************************************************************
 * 功能描述: 根据车头车尾位置计算限速，两者取最小
 * 输入参数: 无
 * 输出参数: 无
 * 返回值:   FLOAT32   等效坡度 千分之一
 *************************************************************************/
UINT16 GetSpeedLimit(UINT32 train_head_loc,UINT32 train_tail_loc)
{
    UINT16 train_head_limit;//列车头部限速
    UINT16 train_tail_limit;//列车尾部限速

    /*查询列车头部限速*/
    for (int i=0;i<g_static_data_csv.speed_limit_csv.length;i++)
    {
        if(i!=g_static_data_csv.speed_limit_csv.length-1)
        {
            if(train_head_loc>=g_static_data_csv.speed_limit_csv.distance[i]&&
               train_head_loc<g_static_data_csv.speed_limit_csv.distance[i+1])
            {
                train_head_limit=g_static_data_csv.speed_limit_csv.speed_limit[i];
                break;
            }
        }
        else
        {
            train_head_limit=g_static_data_csv.speed_limit_csv.speed_limit[i];
            break;
        }
    }

    /*查询列车尾部限速*/
    for (int i=0;i<g_static_data_csv.speed_limit_csv.length;i++)
    {
        if (i != g_static_data_csv.speed_limit_csv.length - 1)
        {
            if (train_tail_loc >= g_static_data_csv.speed_limit_csv.distance[i] &&
                train_tail_loc < g_static_data_csv.speed_limit_csv.distance[i + 1]) {
                train_tail_limit = g_static_data_csv.speed_limit_csv.speed_limit[i];
                break;
            }
        }
        else
        {
            train_tail_limit = g_static_data_csv.speed_limit_csv.speed_limit[i];
            break;
        }
    }
    /*min*/
    if(train_head_limit < train_tail_limit)
    {
        return train_head_limit*1000/36;
    }
    else
    {
        return train_tail_limit*1000/36;
    }

}

/*************************************************************************
 * 功能描述: 根据车头位置计算曲线半径
 * 输入参数: 无
 * 输出参数: 无
 * 返回值:   曲线半径
 *************************************************************************/
UINT16 GetCurveRadius(UINT32 train_head_loc)
{
    UINT16 curve_radius;
    for (int i=0;i<g_static_data_csv.curve_radius_csv.length;i++)
    {
        if(i!=g_static_data_csv.curve_radius_csv.length-1)
        {
            if(train_head_loc>=g_static_data_csv.curve_radius_csv.distance[i]
            &&train_head_loc<g_static_data_csv.curve_radius_csv.distance[i+1])
            {
                curve_radius=g_static_data_csv.curve_radius_csv.curve_radius[i];
                break;
            }
        }
        else
        {
            curve_radius=g_static_data_csv.curve_radius_csv.curve_radius[i];
            break;
        }

    }
    return curve_radius;
}

/*************************************************************************
 * 功能描述: 线路静态数据初始化
 * 输入参数: 无
 * 输出参数: 无
 * 返回值:   UINT8   1:成功 0：失败
 *************************************************************************/
UINT8 LineStaticDataInit()
{
    UINT8 result;
    g_line_param.discrete_size=5;//m
    /*区间长度*/
    g_line_param.line_length= g_static_data_csv.station_csv.end_distance[g_static_data_csv.station_csv.length-2];
    g_line_param.discrete_num=g_line_param.line_length/g_line_param.discrete_size;
    /*坡度*/
    UINT32 train_head_loc;
    UINT32 train_tail_loc;
    for(int i=0;i<g_line_param.discrete_num;i++)
    {
        train_head_loc=i*g_line_param.discrete_size;
        train_tail_loc=GetTrainTailLocation(train_head_loc,g_train_param.train_length/100);
        /*坡度*/
        g_line_param.gradient[i] = GetEquivalentGradient(train_head_loc,train_tail_loc);
        /*限速*/
        g_line_param.limit[i] = GetSpeedLimit(train_head_loc,train_tail_loc);
        /*曲线半径*/
        g_line_param.curve_radius[i]=GetCurveRadius(train_head_loc);
    }
    return 1;
}

/*************************************************************************
 * 功能描述: 初始化线路静态数据、列车静态数据
 * 输入参数: 无
 * 输出参数: 无
 * 返回值:   UINT8   1:读取成功 0：读取失败
 *************************************************************************/
UINT8 StaticDataInit()
{
    UINT8 result;
    /*初始化列车数据*/
    result=TrainStaticDataInit();
    if(result==0)
        return result;
    /*初始化线路数据*/
    result=LineStaticDataInit();
    if(result==0)
        return result;
    return result;

}

void JestForTest()
{
    /*载荷*/
    g_train_param.aw[0] = 222; //AW0载荷 222t
    g_train_param.aw[1] = 0;   //AW1载荷 无数据
    g_train_param.aw[2] = 334; //AW2载荷 334t
    g_train_param.aw[3] = 373; //AW3载荷 373t
    /*戴维斯基本阻力系数，分别对应AW0、AW1、AW2、AW3载荷下a、b、c系数值 f=a+b*v+c*v*v 速度单位km/h 力单位kN*/
    g_train_param.a[0] = 3.08384;
    g_train_param.a[1] = 0;
    g_train_param.a[2] = 3.5580656;
    g_train_param.a[3] = 3.74469632;
    g_train_param.b[0] = 0.01462444;
    g_train_param.b[1] = 0;
    g_train_param.b[2] = 0.021424811;
    g_train_param.b[3] = 0.024101084;
    g_train_param.c[0] = 0.000647994;
    g_train_param.c[1] = 0;
    g_train_param.c[2] = 0.000647994;
    g_train_param.c[3] = 0.000647994;
    /*牵引特性曲线，不同速度下（km/h）、AW0、AW2、AW3载荷下，列车能够输出的最大牵引加速度cm/s^2 */
    g_train_param.traction_num = 121;
    UINT16 t_v[] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120 };
    UINT16 t_aw0[] = { 108,108,108,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,108,107,105,103,102,100,99,97,96,95,93,92,91,89,88,87,86,85,83,82,81,80,79,78,77,76,75,74,73,73,72,71,70,69,67,66,64,63,62,60,59,58,56,55,54,53,52,50,49,48,47,46,45,44 };
    UINT16 t_aw2[] = { 108,108,108,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,110,110,110,110,110,110,110,110,110,108,106,103,101,99,97,95,94,92,90,88,87,85,84,82,81,80,78,77,76,75,73,72,71,70,69,68,67,66,65,64,63,62,62,61,60,59,58,57,57,56,55,55,54,53,52,52,51,51,50,49,49,48,47,46,45,44,43,42,41,40,39,38,37,37,36,35,34,33,33,32,31,31 };
    UINT16 t_aw3[] = { 105,105,105,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,108,108,108,108,108,108,108,108,106,104,101,99,97,95,93,91,90,88,86,85,83,82,80,79,77,76,75,74,72,71,70,69,68,67,66,65,64,63,62,61,60,59,59,58,57,56,55,55,54,53,53,52,51,51,50,49,49,48,47,47,46,46,45,44,43,42,41,40,39,39,38,37,36,35,34,34,33,32,31,31,30,29,29 };

    memcpy(g_train_param.traction_v, t_v, g_train_param.traction_num * sizeof(UINT16));
    memcpy(g_train_param.traction_a0, t_aw0, g_train_param.traction_num * sizeof(UINT16));
    memcpy(g_train_param.traction_a2, t_aw2, g_train_param.traction_num * sizeof(UINT16));
    memcpy(g_train_param.traction_a3, t_aw3, g_train_param.traction_num * sizeof(UINT16));
    /*制动特性曲线,不同速度下（km/h）、AW0、AW2、AW3载荷下，列车能够输出的最大制动加速度cm/s^2 */
    g_train_param.braking_num = 121;
    UINT16 b_v[] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120 };
    UINT16 b_aw0[] = { 104,104,104,104,104,104,104,104,104,104,104,104,104,104,104,104,104,104,104,104,104,104,104,104,104,104,104,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,108,108,108,108,108,108,108,108,108,108,108,108,108,109,109,109,109,109,109,109,109,109,109,109,109,110,110,110,110,110,110,110,110,110 };
    UINT16 b_aw2[] = { 104,104,104,104,104,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,109,109,109,109,109,109,109,109,109,109,109 };
    UINT16 b_aw3[] = { 104,104,104,104,104,104,104,104,104,104,104,104,104,104,104,104,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,109,109,109 };
    memcpy(g_train_param.braking_v, b_v, g_train_param.braking_num * sizeof(UINT16));
    memcpy(g_train_param.braking_a0, b_aw0, g_train_param.braking_num * sizeof(UINT16));
    memcpy(g_train_param.braking_a2, b_aw2, g_train_param.braking_num * sizeof(UINT16));
    memcpy(g_train_param.braking_a3, b_aw3, g_train_param.braking_num * sizeof(UINT16));
    /*以下为测试数据*/
    g_aw_id = 0;//AW0载荷
    g_train_param.train_length = 7000;//车长70m

    g_line_param.line_length = 1367;
    for (int i = 0; i < g_line_param.discrete_size; i++)
    {
        if (i <= 49)
        {
            g_line_param.limit[i] = 1289;
        }
        else if (i > 49 && i <= 102)
        {
            g_line_param.limit[i] = 2261;
        }
        else if (i > 102 && i <= 160)
        {
            g_line_param.limit[i] = 2956;
        }
        else if (i > 160 && i <= 411)
        {
            g_line_param.limit[i] = 2261;
        }
        else if (i > 411 && i <= 912)
        {
            g_line_param.limit[i] = 2483;
        }
        else if (i > 912 && i <= 1008)
        {
            g_line_param.limit[i] = 2261;
        }
        else if (i > 1008 && i <= 1174)
        {
            g_line_param.limit[i] = 2070;
        }
        else
        {
            g_line_param.limit[i] = 1289;
        }
    }
    FLOAT32 gradient_debug[] = {-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2.0012, -2.00988, -2.02688, -2.0522, -2.08588, -2.12788, -2.1782, -2.23688, -2.30387, -2.3792, -2.46287, -2.55487, -2.6552, -2.76388, -2.88088, -3.0062, -3.13987, -3.28187, -3.4322, -3.59087, -3.75787, -3.9332, -4.11688, -4.30888, -4.5092, -4.71788, -4.93488, -5.1602, -5.39388, -5.63588, -5.8862, -6.14488, -6.41187, -6.6872, -6.97087, -7.26288, -7.5632, -7.87187, -8.18888, -8.5142, -8.84345, -9.17685, -9.51027, -9.8437, -10.1771, -10.5105, -10.844, -11.1774, -11.5108, -11.8442, -12.1776, -12.511, -12.8444, -13.1778, -13.5113, -13.8447, -14.1781, -14.5115, -14.8449, -15.1784, -15.5118, -15.8452, -16.1786, -16.512, -16.8454, -17.1789, -17.5123, -17.8457, -18.1791, -18.5125, -18.8459, -19.1793, -19.5128, -19.8462, -20.1796, -20.513, -20.8465, -21.1798, -21.5133, -21.8467, -22.1787, -22.4991, -22.8112, -23.115, -23.4104, -23.6975, -23.9762, -24.2467, -24.5087, -24.7625, -25.0079, -25.245, -25.4737, -25.6942, -25.9062, -26.11, -26.3054, -26.4925, -26.6712, -26.8417, -27.0037, -27.1575, -27.3029, -27.44, -27.5687, -27.6891, -27.8012, -27.905, -28.0004, -28.0875, -28.1662, -28.2367, -28.2987, -28.3525, -28.3979, -28.435, -28.4637, -28.4841, -28.4962, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.5, -28.4988, -28.4901, -28.4731, -28.4478, -28.4141, -28.3721, -28.3218, -28.2631, -28.1961, -28.1208, -28.0371, -27.9451, -27.8448, -27.7361, -27.6191, -27.4938, -27.3601, -27.2181, -27.0678, -26.9091, -26.7421, -26.5668, -26.3831, -26.1911, -25.9908, -25.7821, -25.5651, -25.3398, -25.1084, -24.8663, -24.6159, -24.3571, -24.09, -23.8146, -23.5309, -23.2388, -22.9384, -22.6296, -22.3125, -21.9871, -21.6546, -21.3212, -20.9878, -20.6544, -20.3209, -19.9875, -19.6541, -19.3207, -18.9873, -18.6539, -18.3204, -17.987, -17.6536, -17.3202, -16.9868, -16.6534, -16.3245, -16.004, -15.6918, -15.388, -15.0925, -14.8054, -14.5265, -14.256, -13.9938, -13.74, -13.4945, -13.2573, -13.0262, -12.8058, -12.5938, -12.39, -12.1946, -12.0075, -11.8287, -11.6583, -11.4963, -11.3425, -11.1971, -11.06, -10.9312, -10.8108, -10.6987, -10.595, -10.4996, -10.4125, -10.3337, -10.2633, -10.2013, -10.1475, -10.1021, -10.065, -10.0363, -10.0158, -10.0037, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -9.99842, -9.99197, -9.98053, -9.96408, -9.94263, -9.91617, -9.88472, -9.84828, -9.80682, -9.76038, -9.70893, -9.65248, -9.59103, -9.52458, -9.45313, -9.37667, -9.29523, -9.20877, -9.11732, -9.02087, -8.91943, -8.81297, -8.70152, -8.58508, -8.46362, -8.33718, -8.20572, -8.06927, -7.92782, -7.78137, -7.6314, -7.4749, -7.31343, -7.1469, -6.9754, -6.7989, -6.61742, -6.4309, -6.23943, -6.04292, -5.84295, -5.64292, -5.44285, -5.2428, -5.04278, -4.84272, -4.64265, -4.44263, -4.24255, -4.04252, -3.84247, -3.6424, -3.44237, -3.24233, -3.04225, -2.84223, -2.64217, -2.4421, -2.24207, -2.04203, -1.84358, -1.65003, -1.46148, -1.27793, -1.09938, -0.925825, -0.757275, -0.593725, -0.435175, -0.281625, -0.1316, 0.0119, 0.150425, 0.2839, 0.4124, 0.5359, 0.654425, 0.7679, 0.876425, 0.979925, 1.07837, 1.1719, 1.26038, 1.34388, 1.4224, 1.4959, 1.56438, 1.6279, 1.68638, 1.7399, 1.7884, 1.83187, 1.8704, 1.9039, 1.93237, 1.9559, 1.9744, 1.98787, 1.9964, 1.9999, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 };
    for (int i = 0; i < g_line_param.discrete_size; i++)
    {
        g_line_param.gradient[i] = gradient_debug[i];
        g_line_param.curve_radius[i] = 0;
    }
}


#pragma region 牵引能耗计算模块

/*************************************************************************
 * 功能描述: 根据速度和载荷计算输出牵引力和电机效率
 * 输入参数: UINT16              speed              速度km/h
 *          UINT8               aw_id              载荷0:AW0 1:AW1 2:AW2 ...
 * 输出参数: FLOAT32*            traction_force     牵引力KN
 *          FLOAT32*            traction_n1        电机效率
 * 返回值:
 *************************************************************************/
void GetTractionForce(UINT16 speed,UINT8 aw_id,FLOAT32* traction_force,FLOAT32* traction_n1)
{
    switch (aw_id)
    {
        //AW0
        case 0:
            for(int i=0;i<g_static_data_csv.dynamics_csv.length;i++)
            {
                if (g_static_data_csv.dynamics_csv.speed[i]>speed)
                {
                    *traction_force=g_static_data_csv.dynamics_csv.traction_aw0[i-1];
                    *traction_n1=g_static_data_csv.dynamics_csv.traction_motor_aw0[i-1];
                    break;
                }
            }
        case 1:
            break;
        default:
            *traction_force=0;
            *traction_n1=0;
            break;
    }
}

/*************************************************************************
 * 功能描述: 根据速度和载荷计算输出制动力和效率
 * 输入参数: UINT16              speed              速度km/h
 *          UINT8               aw_id              载荷0:AW0 1:AW1 2:AW2 ...
 * 输出参数: FLOAT32*            brake_force        制动力KN
 *          FLOAT32*            brake_n1           电机效率
 * 返回值:
 *************************************************************************/
void GetBrakeForce(UINT16 speed,UINT8 aw_id,FLOAT32* brake_force,FLOAT32* brake_n1)
{
    switch (aw_id)
    {
        //AW0
        case 0:
            for(int i=0;i<g_static_data_csv.dynamics_csv.length;i++)
            {
                if (g_static_data_csv.dynamics_csv.speed[i]>speed)
                {
                    *brake_force=g_static_data_csv.dynamics_csv.brake_aw0[i-1];
                    *brake_n1=g_static_data_csv.dynamics_csv.brake_motor_aw0[i-1];
                    break;
                }
            }
        case 1:
            break;
        default:
            *brake_force=0;
            *brake_n1=0;
            break;
    }
}

/*************************************************************************
 * 功能描述: 根据模型计算能耗
 * 输入参数: UINT16          speed           速度km/h
 *          UINT8           level_flag      级位标识 1：牵引 4：制动
 *          UINT8           level_output    级位输出0-100
 *          UINT32          distance        走行距离
 * 输出参数: 无
 * 返回值:  UINT32 焦耳
 *************************************************************************/
UINT32 CalEnergyByMode(UINT16 speed,UINT8 level_flag,UINT8 level_output,UINT32 distance)
{
    UINT32 W; //焦耳
    FLOAT32 traction_force,brake_force,n1,n2,n3,n4;//牵引力 制动力 牵引电机效率 变压器效率 变流器效率 齿轮箱效率
    n2=0.95f;
    n3=0.98f;
    n4=0.97f;
    switch (level_flag)
    {
        //牵引
        case 1:
            GetTractionForce(speed,g_aw_id,&traction_force,&n1);//计算当前输出的牵引力kN和电机效率
            W=(UINT32)(1000.0*traction_force*level_output/100*distance/n1/n2/n3/n4);
            break;
        case 4:
            GetBrakeForce(speed,g_aw_id,&brake_force,&n1);//计算当前输出的制动力kN和电机效率
            W=(UINT32)(1000.0*brake_force*level_output/100*distance*n1*n2*n3*n4);
            break;
        default:
            W=0;
            break;
    }
    return W;
}

/*************************************************************************
 * 功能描述: 根据电压电流计算能耗
 * 输入参数: UINT32          voltage           电压v
 *          UINT32          current           电流A
 *          FLOAT32         cycle_time        周期s
 * 输出参数: 无
 * 返回值:  UINT32 焦耳
 *************************************************************************/
UINT32 CalEnergyByUI(UINT32 voltage,UINT32 current,FLOAT32 cycle_time)
{
    UINT32 W; //焦耳
    W=(UINT32)(1.0*voltage*current*cycle_time);
    return W;

}

#pragma endregion