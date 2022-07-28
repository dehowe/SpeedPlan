#include "SpeedPlan.h"

TRAIN_PARAMETER				g_train_param;						// �г�����
UINT16						g_aw_id;							// �غ�
LINE_PARAMETER              g_line_param;                       // ��·����
/*�ٶȹ滮��ر���*/
FLOAT32* gradient = NULL;						 // ��ʼ�������¶ȴ洢ָ��;
UINT32* curve_radius = NULL;					 // ��ʼ���������߰뾶�洢ָ��;
UINT32* speed_limit_location = NULL;			 // ��ʼ������ת����λ�ô洢ָ��;
UINT16* speed_limit = NULL;						 // ��ʼ������ת�������ٴ洢ָ��;
UINT16* speed_curve_offline = NULL;				 // ��ʼ�������Ż��ٶȴ洢ָ��;
UINT16* tartget_speed_curve = NULL;				 // ��ʼ���Ż��ٶȴ洢ָ��;
UINT16* speed_limit_max = NULL;					 // ��ʼ������ٶȴ洢ָ��;
UINT16* speed_limit_min = NULL;					 // ��ʼ����С�ٶȴ洢ָ��;
UINT16* speed_limit_mmax = NULL;				 // ��ʼ���������ٴ洢ָ��;
UINT16* level_output_flag = NULL;				 // ��ʼ����λ�����ʶ1��ǣ����2������;3���ƶ�;
FLOAT32* plan_time = NULL;						 // ��ʼ������ɢ����Ҫ�ﵽ������ʱ��;
UINT32	interval_length;						// ��һ���䳤��m
UINT32	interval_length_cm;						// ��һ���䳤��cm
UINT32	target_time_offline;					// �������Ŀ������ʱ��
UINT32	target_time_online;						// �����Ż�Ŀ������ʱ��
UINT16	dim;									// ��ģ��ɢά��
UINT8	limit_num;								// �����л������
UINT16  solution_num = 50;						// ��ռ��С
UINT16  discrete_size = 100;					// ��ɢ����
FLOAT32 optimal_time_offline = 0;				// �����Ż�����ʱ��
TRAIN_LOACTION_STRU train_start_loc;
TRAIN_LOACTION_STRU train_start_loc;			// �г���ʼλ��
UINT16 remain_section_length;					// ������еȼ����ɢ���ʣ�೤��

/*************************************************************************
* ��������: ��������㷨���
* �������: ��
* �������: ��
* ����ֵ:   ��
*************************************************************************/
void SpeedPlanOffline()
{
	UINT32 start = clock();
	//�����Ż���������
	target_time_offline = 120;
	//��������Ԥ����
	gradient = (FLOAT32*)malloc(MAX_INTERVAL_SAMPLING * sizeof(FLOAT32));				// ��ʼ�������¶ȴ洢ָ��;
	curve_radius = (UINT32*)malloc(MAX_INTERVAL_SAMPLING * sizeof(UINT32));				// ��ʼ���������߰뾶�洢ָ��;
	speed_limit_location = (UINT32*)malloc(MAX_LIMIT_POINT * sizeof(UINT32));			// ��ʼ������ת����λ�ô洢ָ��;
	speed_limit = (UINT16*)malloc(MAX_LIMIT_POINT * sizeof(UINT16));					// ��ʼ������ת�������ٴ洢ָ��;
	speed_curve_offline = (UINT16*)malloc(MAX_SPEED_CURVE * sizeof(UINT16));			// ��ʼ�������Ż��ٶȴ洢ָ��;
	tartget_speed_curve = (UINT16*)malloc(MAX_SPEED_CURVE * sizeof(UINT16));			// ��ʼ���Ż��ٶȴ洢ָ��;
	speed_limit_max = (UINT16*)malloc(MAX_SPEED_CURVE * sizeof(UINT16));				// ��ʼ������ٶȴ洢ָ��;
	speed_limit_min = (UINT16*)malloc(MAX_SPEED_CURVE * sizeof(UINT16));				// ��ʼ����С�ٶȴ洢ָ��;
	speed_limit_mmax = (UINT16*)malloc(MAX_SPEED_CURVE * sizeof(UINT16));				// ��ʼ���������ٴ洢ָ��;
	level_output_flag = (UINT16*)malloc(MAX_SPEED_CURVE * sizeof(UINT16));              // ��ʼ����λ�����ʶ1��ǣ����2������;3���ƶ���;
	plan_time = (FLOAT32*)malloc(MAX_SPEED_CURVE * sizeof(FLOAT32));					// ��ʼ������ɢ����Ҫ�ﵽ������ʱ��;
	
	//����׼��		
	GetBaseDataReady();
																				
	//�������
	//���ڲ�����ʼ��
	memset(level_output_flag, 0, MAX_SPEED_CURVE * sizeof(UINT16));
	dim = GetOptimalSpeedOffline(speed_curve_offline, discrete_size, speed_limit_max, speed_limit_min, speed_limit_mmax,
		interval_length_cm, speed_limit_location, speed_limit, limit_num, target_time_offline, solution_num);
	UINT32 finish = clock();
	printf("��������������ʱ�� % d ms\n", (finish - start) / 1000);
}


/*************************************************************************
* ��������: ����Ԥ�����������䳤�ȡ����١��¶ȡ����߰뾶����Ϣ
* �������:
*		const EMAP_DATA&			emap				���ӵ�ͼ����
*		TRAIN_LOACTION_STRU			train_head_loc		�г�ͷλ��
*		TRAIN_LOACTION_STRU			train_tail_loc		�г�βλ��
*		UINT8						direction			���з���
*		UINT16						park_area_id		Ŀ��ͣ������id
*		UINT16						park_point_index	Ŀ��ͣ��������
* �������:
* ����ֵ:   0:��ѯʧ�� 1:��ѯ�ɹ�
*************************************************************************/
UINT8 GetBaseDataReady()
{
	UINT8  result = 1;						/*��������ֵ*/
	UINT16 speed_limit_last = 0;			/*��һ����ֵ��ʱ����*/
	UINT32 k = 0;							/*ѭ������*/
	INT32  gradient_equivalent;				/*��Ч�¶�ֵ*/
	UINT16 curve_radius_id;					/*���߰뾶id��ʱ����*/
	UINT32 curve_radius_value;				/*���߰뾶ֵ*/

	if (0 != g_line_param.interval_length)
	{
		/*�����м�link���С��г�λ�á���һͣ����������䳤��*/
		interval_length = g_line_param.interval_length;
		interval_length_cm = interval_length * 100;

		/*��������λ�����٣��ҵ������л��㼰��Ӧ����*/
		speed_limit_last = g_line_param.limit[0];
		for (UINT32 i = 1; i < interval_length; i++)
		{
			if (g_line_param.limit[i] != speed_limit_last)//�����л�
			{
				/*��¼�л���λ�ú�����ֵ*/
				speed_limit_location[k] = (i - 1) * 100;
				speed_limit[k] = speed_limit_last;
				/*������һ����*/
				speed_limit_last = g_line_param.limit[i];
				k += 1;
			}
			else
			{
				continue;
			}
		}
		speed_limit_location[k] = (interval_length) * 100;
		speed_limit[k] = g_line_param.limit[interval_length - 1];
		limit_num = k+1;
		for (INT32 i = 0; i < limit_num; i++)
		{
			printf("����ת������㣺λ�ã�%d,���٣�%d\n", speed_limit_location[i], speed_limit[i]);
		}
		for (INT32 i = 0; i < interval_length; i++)
		{
			gradient[i] = g_line_param.gradient[i]; //�¶�
			curve_radius[i] = g_line_param.curve_radius[i];//���߰뾶
			printf("λ�ã�%d,�¶ȣ�%f,���߰뾶��%d\n", i*100, gradient[i], curve_radius[i]);
		}
	}

	/*�ͷ���������ڴ�, ��ֹ�ڴ�й¶*/
	return result;
}

/*************************************************************************
* ��������: ���߼�����һ�����Ż��ٶ�
* �������:
*		UINT16						discrete_size			������ɢ��С
*		UINT32						section_length			���䳤��cm
*		UINT32						speed_limit_location[]	�����л���λ��
*		UINT16						speed_limit[]			��Ӧ�л�������ֵ
*		UINT16						speed_limit_length		�����л������
*		UINT16						target_time				����Ŀ������ʱ��
*		UINT16						solution_num			��ռ��С
* �������:
*		UINT16*						speed_curve				�ٶ�����
*		UINT16*						speed_limit_max			����ٶ���������
*		UINT16*						speed_limit_min			�����ٶ���������
*		UINT16*						speed_limit_mmax		�����ٶ���������
* ����ֵ:
*		UINT16						dim						��ռ�ά��
*************************************************************************/
UINT16 GetOptimalSpeedOffline(UINT16* speed_curve, UINT16 discrete_size, UINT16* speed_limit_max, UINT16* speed_limit_min, UINT16* speed_limit_mmax, UINT32 section_length, UINT32 speed_limit_location[], UINT16 speed_limit[], UINT16 speed_limit_length, UINT16 target_time, UINT16 solution_num)
{
	//��ѧ��ģ
	UINT16 dim = (UINT16)ceil(section_length * 1.0 / discrete_size)-1;//���ȼ������ά��
	remain_section_length = section_length - dim * discrete_size;
	Modeling(speed_limit_max, speed_limit_min, speed_limit_mmax, discrete_size, dim, solution_num, speed_limit_location, speed_limit, speed_limit_length, target_time);
	//���ݵ�ǰλ������
	UINT32* lower_bound = (UINT32*)malloc(5 * sizeof(UINT32));//��ʼ������±߽�洢ָ��;
	if (NULL == lower_bound)
		return dim;
	UINT32* upper_bound = (UINT32*)malloc(5 * sizeof(UINT32));//��ʼ������ϱ߽�洢ָ��;
	if (NULL == upper_bound)
		return dim;
	UINT8* switch_flag = (UINT8*)malloc(5 * sizeof(UINT8));//��ʼ�������л���ʶ�洢ָ��;
	if (NULL == switch_flag)
		return dim;
	UINT8 bound_size = GetBounderOffline(lower_bound, upper_bound, switch_flag);
	for (INT32 i = 0; i < bound_size; i++)
	{
		printf("�½�:%d;�Ͻ�:%d\n", lower_bound[i], upper_bound[i]);
	}
	//�����㷨���
	GWO_Offline(speed_curve, speed_limit_max, speed_limit_min, dim, target_time, discrete_size, lower_bound, upper_bound, switch_flag, bound_size);
	free(lower_bound);
	free(upper_bound);
	free(switch_flag);
	return dim;

}




/*************************************************************************
  * ��������: ��ģ
  * �������:
  *		UINT16						discrete_size			������ɢ��С
  *		UINT16						dim						��ռ�ά��
  *		UINT16						solution_num			��ռ��С
  *		UINT32						speed_limit_loc[]		��·����-��Ӧλ��
  *		UINT16						speed_limit[]			��·����-��Ӧ����ֵ
  *		UINT16						speed_limit_length		��·���ٴ洢�����С
  *		UINT16						target_time				Ŀ������ʱ��
  * �������:
  *		UINT16*						speed_limit_max			����ٶ�����
  *		UINT16*						speed_limit_min			�����ٶ�����
  *		UINT16*						speed_limit_mmax		������������
  * ����ֵ:
  *************************************************************************/
void Modeling(UINT16* speed_limit_max, UINT16* speed_limit_min, UINT16* speed_limit_mmax, UINT16 discrete_size, UINT16 dim,
	UINT16 solution_num, UINT32 speed_limit_loc[], UINT16 speed_limit[], UINT16 speed_limit_length, UINT16 target_time)
{
	UINT16 j = 0;						//���ٱ仯��������ʼ��
	UINT16 p = 0;						//�����½���������ʼ��
	UINT32 location_index;
	UINT16* speed_limit_temp = NULL;	//������ɢ�洢ָ���ʼ��
	UINT16* limit_fall_begin = NULL;	//�����½���ʼ�ٶ�
	UINT16* limit_fall_end = NULL;		//�����½������ٶ�
	UINT16* limit_fall_index = NULL;	//�����½���Ӧ��ɢ����
	speed_limit_temp = (UINT16*)malloc(MAX_INTERVAL_SAMPLING * sizeof(UINT16));
	limit_fall_begin = (UINT16*)malloc(MAX_LIMIT_POINT * sizeof(UINT16));
	limit_fall_end = (UINT16*)malloc(MAX_LIMIT_POINT * sizeof(UINT16));
	limit_fall_index = (UINT16*)malloc(MAX_LIMIT_POINT * sizeof(UINT16));
	/*�����ڴ���Ч����*/
	if (NULL == speed_limit_temp || NULL == limit_fall_begin || NULL == limit_fall_end || NULL == limit_fall_index)
		return;
	/*��ȡǰ�������½�*/
	for (UINT16 i = 0; i <= dim; i++)
	{
		speed_limit_temp[i] = speed_limit[j];
		speed_limit_mmax[i] = speed_limit[j];
		location_index = (i + 1) * discrete_size;
		if (location_index < speed_limit_loc[j])
		{
			continue;
		}
		else
		{
			j++;
			if (j < speed_limit_length)
			{
				if (speed_limit[j] < speed_limit[j - 1])
				{
					limit_fall_begin[p] = GetEbiEnd(speed_limit[j], speed_limit_temp, i);
					limit_fall_end[p] = speed_limit[j];
					limit_fall_index[p] = i;
					p++;
				}
			}
			else
			{
				limit_fall_begin[p] = GetEbiEnd(0, speed_limit_temp, i);
				limit_fall_end[p] = 0;
				limit_fall_index[p] = i - 1;
				p++;
			}
		}
	}
	speed_limit_temp[dim] = 0;
	speed_limit_mmax[dim] = 0;
	/*���������½�����Ŀ���������ٶ�*/
	for (UINT16 i = 0; i < p; i++)
	{
		GetEBI(limit_fall_end[i], limit_fall_begin[i], limit_fall_index[i], speed_limit_temp, discrete_size);
	}
	for (UINT16 i = 0; i <= dim; i++)
	{
		speed_limit_max[i] = speed_limit_temp[i];
		speed_limit_min[i] = 0;
		//printf("�������ٶ�%d\n", speed_limit_max[i]);
	}
	/*�ͷ���������ڴ�, ��ֹ�ڴ�й¶*/
	free(speed_limit_temp);
	free(limit_fall_begin);
	free(limit_fall_end);
	free(limit_fall_index);
}

/*************************************************************************
  * ��������: �����������ټ�������ٶ�
  * �������:
  *		UINT16						speed_begin				���ƿ�ʼ�ٶ�
  *		UINT16						speed_end				�����ٶ�
  *		UINT16						index					��ʼ����
  *		UINT16						discrete_size			������ɢ��С
  * �������:
  *		UINT16*						speed_limit				�����ٶ�
  * ����ֵ:
  *************************************************************************/
void GetEBI(UINT16 speed_begin, UINT16 speed_end, UINT16 index, UINT16* speed_limit, UINT16 discrete_size)
{
	UINT16 v_index = speed_begin;
	UINT16 spd;
	UINT16 acc_break;//�ƶ����ٶ�
	while (v_index < speed_end)
	{
		acc_break = (UINT16)(GetBreakAcc(v_index) * SPEED_PLAN_TB_RATIO);
		spd = (UINT16)sqrt(v_index * v_index + 2 * acc_break * discrete_size);
		if (spd < speed_end)
		{
			if (index > 0)
			{
				if (spd < speed_limit[index])
				{
					speed_limit[index] = spd;
				}
				v_index = spd;
				index = index - 1;
			}
			else
			{
				break;
			}
		}
		else
		{
			break;
		}
	}
}

/*************************************************************************
  * ��������: ��������ٶ������յ�Ķ����ٶ�
  * �������:
  *		UINT16						ebi_begin				���ƿ�ʼ�ٶ�
  *		UINT16						index					��ʼ����
  *		UINT16*						speed_limit				����
  * �������:
  * ����ֵ:
  *		UINT16						v_end					�����ٶ�
  *************************************************************************/
UINT16 GetEbiEnd(UINT16 ebi_begin, UINT16* speed_limit, UINT16 index)
{
	UINT16 v_end = ebi_begin;
	for (UINT16 i = 0; i < index - 1; i++)
	{
		if (speed_limit[index - i] >= v_end)
		{
			v_end = speed_limit[index - i];
		}
		else
		{
			break;
		}
	}
	return v_end;
}

/*************************************************************************
  * ��������: �����Ż��㷨������Ź���ת����(�������)
  * �������:
  *		UINT16*						speed_limit_max			����ٶ�����
  *		UINT16*						speed_limit_min			�����ٶ�����
  *		UINT16						dim						��ռ�ά��
  *		UINT16						target_time				Ŀ������ʱ��
  *		UINT16						discrete_size			������ɢ��С
  * �������:
  *		UINT16*						optimal_speed			�Ż��ٶ�
  * ����ֵ:
  *************************************************************************/
void GWO_Offline(UINT16* optimal_speed, UINT16* speed_limit_max, UINT16* speed_limit_min, UINT16 dim, UINT16 target_time,
	UINT16 discrete_size, UINT32* lower_bound, UINT32* upper_bound, UINT8* switch_flag, UINT8 solve_dim)
{
	UINT16 wolves_num = 20;//��Ⱥ��С
	UINT16 iteration = 30;//��������
	//��ʼ��Alpha,Beta,Delta��λ��
	FLOAT32* position_Alpha =(FLOAT32*)malloc(solve_dim * sizeof(FLOAT32));
	FLOAT32* position_Beta = (FLOAT32*)malloc(solve_dim * sizeof(FLOAT32));
	FLOAT32* position_Delta = (FLOAT32*)malloc(solve_dim * sizeof(FLOAT32));
	//��ʼ��Alpha,Beta,Delta����Ӧ��
	UINT16 score_Alpha = 65535;
	UINT16 score_Beta = 65535;
	UINT16 score_Delta = 65535;
	UINT16 fitness;//��Ӧ��
	FLOAT32 convergence_factor = 1;//�������ӳ�ʼֵ
	//����ΪGWO�����̲�����Ԥ�ȳ�ʼ��
	FLOAT32 a;
	FLOAT32 A1, A2, A3;
	FLOAT32 C1, C2, C3;
	FLOAT32 X1;
	FLOAT32 X2;
	FLOAT32 X3;
	FLOAT32 D_alpha;
	FLOAT32 D_beta;
	FLOAT32 D_delta;
	UINT16* OptimalSpd = NULL;
	OptimalSpd = (UINT16*)malloc(MAX_SPEED_CURVE * sizeof(UINT16));
	/*�����ڴ���Ч����*/
	if (OptimalSpd == NULL)
		return;
	FLOAT32** Positions = NULL;
	Positions = (FLOAT32**)malloc(wolves_num * sizeof(FLOAT32*));
	for (int i = 0; i < wolves_num; i++)
		Positions[i] = (FLOAT32*)malloc(solve_dim * sizeof(FLOAT32));

    Initialize(wolves_num, solve_dim, upper_bound, lower_bound, Positions);//��ʼ����Ⱥλ�÷ֲ�

	for (UINT16 i = 0; i < iteration; i++)//ѭ��������
	{
		for (UINT16 m = 0; m < wolves_num; m++)//�ҵ����ε���������Alpha��Beta��Delta��
		{
			//fitness = (UINT16)GetFitnessOffline(OptimalSpd, Positions[m], discrete_size, dim, speed_limit_max,speed_limit_min, target_time);//����ÿֻ�ǵ���Ӧ��ֵ����Ŀ��ֵ��СΪĿ��
			fitness = (UINT16)GetFitnessOffline2(OptimalSpd, Positions[m], discrete_size, dim, speed_limit_max, speed_limit_min, target_time, switch_flag, solve_dim);//����ÿֻ�ǵ���Ӧ��ֵ����Ŀ��ֵ��СΪĿ��

			if (fitness <= score_Alpha)
			{
				score_Alpha = fitness;
				position_Alpha = Positions[m];
			}
			if (fitness > score_Alpha && fitness < score_Beta)
			{
				score_Beta = fitness;
				position_Beta = Positions[m];
			}
			if (fitness > score_Alpha && fitness > score_Beta && fitness < score_Delta)
			{
				score_Delta = fitness;
				position_Delta = Positions[m];
			}
		}
		a = (FLOAT32)(convergence_factor - (2.0 / iteration) * i);//�����������ŵ������̽��У��ɳ�ʼֵ���Լ�С��0
		if (a < 0)
		{
			a = 0;
		}
		for (UINT16 m = 0; m < wolves_num; m++)
		{
			for (UINT16 n = 0; n < solve_dim; n++)
			{
				//r2 = rand();
				A1 = 2 * a * (FLOAT32)rand() / RAND_MAX - a;//����ϵ��A��Equation (3.3)
				C1 = 2 * (FLOAT32)rand() / RAND_MAX;//����ϵ��C��Equation (3.4)
				//Alpha��λ�ø���
				D_alpha = abs(C1 * position_Alpha[n] - Positions[m][n]);
				X1 = position_Alpha[n] - A1 * D_alpha;

				A2 = 2 * a * (FLOAT32)rand() / RAND_MAX - a;
				C2 = 2 * (FLOAT32)rand() / RAND_MAX;
				//Beta��λ�ø���
				D_beta = abs(C2 * position_Beta[n] - Positions[m][n]);
				X2 = position_Beta[n] - A2 * D_beta;

				A3 = 2 * a * (FLOAT32)rand() / RAND_MAX - a;
				C3 = 2 * (FLOAT32)rand() / RAND_MAX;
				//Delta��λ�ø���
				D_delta = abs(C3 * position_Delta[n] - Positions[m][n]);
				X3 = position_Delta[n] - A3 * D_delta;

				Positions[m][n] = (X1 + X2 + X3) / 3;//λ�ø��£�Equation (3.7)

				if (Positions[m][n] > upper_bound[n])//�߽��ж�
				{
					Positions[m][n] = (FLOAT32)upper_bound[n];
				}
				if (Positions[m][n] < lower_bound[n])
				{
					Positions[m][n] = (FLOAT32)lower_bound[n];
				}
				/*if (Positions[m][3] < Positions[m][2])
				{
					Positions[m][3] = Positions[m][2];
				}*/
			}

		}
	}
	//����ÿֻ�ǵ���Ӧ��ֵ����Ŀ��ֵ��СΪĿ��
	//fitness = (UINT16)GetFitnessOffline(OptimalSpd, position_Alpha, discrete_size, dim, speed_limit_max, speed_limit_min, target_time);
	fitness = (UINT16)GetFitnessOffline2(OptimalSpd, position_Alpha, discrete_size, dim, speed_limit_max, speed_limit_min, target_time, switch_flag, solve_dim);
	for (UINT16 i = 0; i < solve_dim; i++)
	{
		printf("���Ž⣺%f\n", position_Alpha[i]);
	}
	for (UINT16 i = 0; i < dim + 1; i++)//������Ž�
	{
		optimal_speed[i] = OptimalSpd[i];
		printf("λ�ã�%dm,�Ż��ٶȣ�%dcm/s,���������%d\n", i, OptimalSpd[i], level_output_flag[i]);
	}

	printf("���䳤�ȣ�%d,Ŀ������ʱ�֣�%d,�Ż�����ʱ�֣�%f\n", interval_length, target_time, plan_time[dim - 1]);
	
	for (UINT16 i = 0; i < dim + 1; i++)//������Ž�
	{
		optimal_speed[i] = OptimalSpd[i];
	}
	free(OptimalSpd);


}


/*************************************************************************
  * ��������: ���ݲ������½磬��ʼ����Ⱥλ��
  * �������:
  *		UINT16						wolves_num				��Ⱥ����
  *		UINT8						solve_dim				��ά��
  *		UINT32						upper_bound[]			Լ���Ͻ�
  *		UINT32						lower_bound[]			Լ���½�
  * �������:
  * ����ֵ:
  *		vector<vector<FLOAT32>>		positions				��ʼ��ռ�
  *************************************************************************/
void Initialize(UINT16 wolves_num, UINT8 solve_dim, UINT32 upper_bound[], UINT32 lower_bound[],FLOAT32** Positions)
{
	srand((UINT32)(time(0)));//�޸�����
	for (UINT16 i = 0; i < wolves_num; i++)
	{
		for (UINT16 j = 0; j < solve_dim; j++)
		{
			Positions[i][j] = (FLOAT32)(rand() % (upper_bound[j] - lower_bound[j] + 1) + lower_bound[j]);
		}
	}
}

/*************************************************************************
  * ��������: ������Ӧ�Ⱥ���(�������)
  * �������:
  *		vector<FLOAT32>				position				��ռ�
  *		UINT16						discrete_size			������ɢ��С
  *		UINT16						dim						��ռ�ά��
  *		UINT16*						speed_limit_max			����ٶ�����
  *		UINT16*						speed_limit_min			�����ٶ�����
  *		UINT16						target_time				Ŀ������ʱ��
  * �������:
  *		UINT16*						optimal_speed			�Ż��ٶ�
  * ����ֵ:
  *		FLOAT32						fitness					Ŀ�꺯��ֵ
  *************************************************************************/
FLOAT32 GetFitnessOffline2(UINT16* optimal_speed, FLOAT32* position, UINT16 discrete_size, UINT16 dim, UINT16* speed_limit_max, UINT16* speed_limit_min, UINT16 target_time, UINT8 switch_flag[], UINT8 switch_num)
{
	FLOAT32 v_index = 0.01f; //�����ٶ�
	UINT32 train_weight = (UINT32)CalTrainWeight();//����
	FLOAT32 time_sum = 0;//����ʱ���ܺ�
	FLOAT32 energy_sum = 0;//�ܺ��ܺ�
	FLOAT32 acc_gradient;//�¶ȸ��Ӽ��ٶ�
	FLOAT32 acc_qxbj;//���߰뾶���Ӽ��ٶ�
	FLOAT32 acc_w;//�����������Ӽ��ٶ�
	FLOAT32 acc_traction;//ǣ�����ٶ�
	FLOAT32 acc_index;//��Ч���ٶ�
	FLOAT32 v_next;//��һ�ٶ�
	FLOAT32 P = 0.85f;//����ʱ�ֳͷ�ϵ��
	FLOAT32 fitness;//Ŀ�꺯��ֵ
	UINT8 pos_index = 0;//������
	for (UINT16 i = 0; i < dim; i++)
	{
		acc_gradient = GetGradientAcc(i * discrete_size);
		acc_qxbj = GetCurveRadiusAcc(i * discrete_size);
		acc_w = GetResistanceAcc((UINT16)v_index);
		acc_traction = GetTractionAcc((UINT16)v_index) * SPEED_PLAN_TB_RATIO;
		if (i * discrete_size <= position[pos_index])
		{
			if (switch_flag[pos_index] == 1)//ǣ��-����
			{
				acc_index = acc_traction - acc_w - acc_gradient - acc_qxbj;
				v_next = sqrt(v_index * v_index + 2 * acc_index * discrete_size);
				if (v_next >= speed_limit_max[i + 1])//��һ�滮�ٶȳ���
				{
					acc_index = (speed_limit_max[i + 1] * speed_limit_max[i + 1] - v_index * v_index) / (2 * discrete_size);
					acc_traction = acc_index + acc_w + acc_gradient + acc_qxbj;
				}
				if (acc_traction > 0)
				{
					energy_sum = energy_sum + (FLOAT32)train_weight * acc_traction * discrete_size / 10000;
				}
				level_output_flag[i] = 1;
			}
			else if (switch_flag[pos_index] == 3)//�ƶ�-����
			{
			}
			else
			{
				acc_index = -acc_w - acc_gradient - acc_qxbj;
				if (v_index * v_index + 2 * acc_index * discrete_size > 0)
				{
					v_next = sqrt(v_index * v_index + 2 * acc_index * discrete_size);
				}
				else
				{
					v_next = 0.1f;
				}
				level_output_flag[i] = 2;
			}
			//������һ�����л���
			if ((i + 1) * discrete_size > position[pos_index] && pos_index < switch_num - 1)
			{
				pos_index += 1;
			}
		}
		else
		{
			acc_index = -acc_w - acc_gradient - acc_qxbj;
			if (v_index * v_index + 2 * acc_index * discrete_size > 0)
			{
				v_next = sqrt(v_index * v_index + 2 * acc_index * discrete_size);
			}
			else
			{
				v_next = 0.1f;
			}
			level_output_flag[i] = 2;
		}
		//�߽�Լ��
		if (v_next > speed_limit_max[i + 1])
		{
			v_next = speed_limit_max[i + 1];
			//�����ǰ����ǣ���׶Σ��滮�ٶȴ��ڷ����ٶȣ����ʻ�׶��л�Ϊ���С�
			if (switch_flag[pos_index] == 1 && i * discrete_size <= position[pos_index])
			{
				level_output_flag[i] = 2;
			}
			//�����ʻ�׶��л�Ϊ�ƶ�
			else
			{
				level_output_flag[i] = 3;
			}
		}
		/*if (v_next < SpeedLimitMin[i + 1])
		{
			v_next = SpeedLimitMin[i + 1];
		}*/
		if (i == dim - 1)
		{
			time_sum = time_sum + (FLOAT32)(discrete_size + remain_section_length) / ((v_index + v_next) / 2);
		}
		else
		{
			time_sum = time_sum + (FLOAT32)discrete_size / ((v_index + v_next) / 2);
		}
		optimal_speed[i] = (UINT16)v_index;
		plan_time[i] = time_sum;
		v_index = v_next;
	}
	optimal_speed[dim] = (UINT16)v_index;
	fitness = (1 - P) * energy_sum / 1000000 + P * abs(time_sum - target_time);

	return fitness;
}

/*************************************************************************
  * ��������: �����г�����
  * �������: ��
  * �������: ��
  * ����ֵ:   FLOAT64	train_weight	�г�����, kg
 *************************************************************************/
FLOAT64 CalTrainWeight()
{
	return (FLOAT64)g_train_param.aw[g_aw_id] * 1000.0;	// ������λ�Ƕ�, ��1000ת��Ϊkg
}
/*************************************************************************
 * ��������: ����ATO���������е��������߼���ĳһ�ٶ��µ������ٶ�
 * �������: FLOAT64	speed		�г��ٶ�, km/h
 *			  UINT16    num			��������ɢ����
 *			  UINT16*	curve_v		�����ٶ�������, km/h
 *			  UINT16*	curve_a		���߼��ٶ�������, cm/s/s
 * �������: ��
 * ����ֵ:   FLOAT64	acc			�����ٶ�, cm/s/s
 *************************************************************************/
FLOAT64 GetAccBySpeed(FLOAT64 kph, UINT16 num, const UINT16* curve_v, const UINT16* curve_a)
{
	FLOAT64 acc = 0;					// ���ٶȷ���ֵ��cm/s/s
	if (kph <= curve_v[0])
	{
		acc = curve_a[0];
	}
	else if (kph >= curve_v[num - 1])
	{
		acc = curve_a[num - 1];
	}
	else
	{
		for (UINT16 i = 1; i < num; i++)
		{
			if (kph <= curve_v[i])
			{
				FLOAT64 ratio = (FLOAT64)(curve_v[i] - kph) / (FLOAT64)(curve_v[i] - curve_v[i - 1]);
				acc = curve_a[i - 1] * ratio + curve_a[i] * (1 - ratio);
				break;
			}
		}
	}
	return acc;
}

/*************************************************************************
 * ��������: �����ٶȼ���ǣ�����ٶ�
 * �������:
 *		UINT16						speed					�ٶ� cm/s
 * �������:
 * ����ֵ:
 *		UINT16						acc_traction		    �����ٶ� cm/s^2
 *************************************************************************/
FLOAT32 GetTractionAcc(UINT16 speed)
{
	FLOAT64 kph = 1.0 * speed / 100 * 3.6;
	FLOAT64 acc = 0;							// ���ٶȷ���ֵ��cm/s/s
	const TRAIN_PARAMETER* tp = &g_train_param;	// �г�����ָ��
	switch (g_aw_id)
	{
	case 0:
		acc = GetAccBySpeed(kph, tp->traction_num, tp->traction_v, tp->traction_a0);
		break;
	case 1:
		acc = GetAccBySpeed(kph, tp->traction_num, tp->traction_v, tp->traction_a1);
		break;
	case 2:
		acc = GetAccBySpeed(kph, tp->traction_num, tp->traction_v, tp->traction_a2);
		break;
	case 3:
		acc = GetAccBySpeed(kph, tp->traction_num, tp->traction_v, tp->traction_a3);
		break;
	default:
		acc = GetAccBySpeed(kph, tp->traction_num, tp->traction_v, tp->traction_a0);
		break;
	}
	tp = NULL;
	return (FLOAT32)acc; // ǣ��������ֵ
}

/*************************************************************************
 * ��������: �����ٶȼ����ƶ����ٶ�
 * �������:
 *		UINT16						speed					�ٶ� cm/s
 * �������:
 * ����ֵ:
 *		UINT16						acc_break				�����ٶ� cm/s^2
 *************************************************************************/
FLOAT32 GetBreakAcc(UINT16 speed)
{
	FLOAT64 kph = 1.0 * speed / 100 * 3.6;
	FLOAT64 acc = 0;							// ���ٶȷ���ֵ��cm/s/s
	const TRAIN_PARAMETER* tp = &g_train_param;	// �г�����ָ��
	switch (g_aw_id)
	{
	case 0:
		acc = GetAccBySpeed(kph, tp->braking_num, tp->braking_v, tp->braking_a0);
		break;
	case 1:
		acc = GetAccBySpeed(kph, tp->braking_num, tp->braking_v, tp->braking_a1);
		break;
	case 2:
		acc = GetAccBySpeed(kph, tp->braking_num, tp->braking_v, tp->braking_a2);
		break;
	case 3:
		acc = GetAccBySpeed(kph, tp->braking_num, tp->braking_v, tp->braking_a3);
		break;
	default:
		acc = GetAccBySpeed(kph, tp->braking_num, tp->braking_v, tp->braking_a0);
		break;
	}
	tp = NULL;
	return (FLOAT32)acc; // �ƶ�������ֵ
}
/*************************************************************************
  * ��������: �����ٶȼ�������������Ӽ��ٶ�
  * �������:
  *		UINT16						speed					�ٶ� cm/s
  * �������:
  * ����ֵ:
  *		FLOAT64						acc_basic_resistance	���ٶ� cm/s^2
  *************************************************************************/
FLOAT64 GetResistanceAcc(UINT16 speed)
{
	FLOAT32 acc_basic_resistance = 0;
	const TRAIN_PARAMETER* tp = &g_train_param;
	double kph = 1.0 * speed / 100 * 3.6;
	UINT16 idx = g_aw_id;
	FLOAT64 rf = (tp->a[idx] + tp->b[idx] * kph + tp->c[idx] * kph * kph) * 1000.0; // ��ά˹��ʽ�����KN, ת��ΪN
	tp = NULL;
	acc_basic_resistance = (FLOAT32)(rf / CalTrainWeight() * 100);
	return acc_basic_resistance;
}


/*************************************************************************
  * ��������: ����λ�ü����¶ȸ��Ӽ��ٶ�
  * �������:
  *		UINT32						dispalcement			λ�� cm
  * �������: ��
  * ����ֵ:
  *		FLOAT32						gradient_acc			���ٶ� cm/s^2
  *************************************************************************/
FLOAT32 GetGradientAcc(UINT32 dispalcement)
{
	FLOAT32 gradient_acc = 0;/*�¶ȸ��Ӽ��ٶ�*/
	UINT16 gradient_front_index, gradient_rear_index;/*ǰ����ɢ�¶ȶ�Ӧ����*/
	FLOAT32 gradient_current_index;
	if (dispalcement<0 || dispalcement>interval_length * 100)/*����������*/
	{
		/*ʲôҲ����*/
	}
	else
	{
		gradient_front_index = dispalcement / 100;
		gradient_rear_index = gradient_front_index + 1;
		if ((gradient_front_index > 0 && gradient_front_index < MAX_INTERVAL_SAMPLING)
			&& (gradient_rear_index > 0 && gradient_rear_index < MAX_INTERVAL_SAMPLING))
		{
			gradient_current_index = (FLOAT32)(((FLOAT64)dispalcement - gradient_front_index * 100.0) / 100);
			gradient_acc = ((gradient[gradient_front_index] + (gradient[gradient_front_index] - gradient[gradient_rear_index]) * gradient_current_index) * GRAVITY_ACC / 10);
		}
	}
	return gradient_acc;
}

/*************************************************************************
  * ��������: ����λ�ü������߰뾶���Ӽ��ٶ�
  * �������:
  *		UINT32						dispalcement			λ�� cm
  * �������: ��
  * ����ֵ:
  *		FLOAT32						curve_radius_acc		���ٶ� cm/s^2
  *************************************************************************/
FLOAT32 GetCurveRadiusAcc(UINT32 dispalcement)
{
	FLOAT32 curve_radius_acc = 0;/*�¶ȸ��Ӽ��ٶ�*/
	UINT16 curve_radius_index;
	if (dispalcement<0 || dispalcement>interval_length * 100)/*����������*/
	{
		/*ʲôҲ����*/
	}
	else
	{
		curve_radius_index = dispalcement / 100;
		if (curve_radius_index > 0 && curve_radius_index < MAX_INTERVAL_SAMPLING && curve_radius[curve_radius_index] != 0)
			curve_radius_acc = (FLOAT32)(600.0 / curve_radius[curve_radius_index] * GRAVITY_ACC / 10);
	}
	return curve_radius_acc;
}

/*************************************************************************
  * ��������: �����г���ǰλ�ã����������Ż������±߽�
  * �������:
  * �������:
  *		UINT32*						lower_bound				Լ���½�
  *		UINT32*						upper_bound				Լ���Ͻ�
  *		UINT8*						switch_flag				�����л���ʶ
  * ����ֵ:
  *		UINT8												��ά��
  *************************************************************************/
UINT8 GetBounderOffline(UINT32* lower_bound, UINT32* upper_bound, UINT8* switch_flag)
{
	UINT32 station_jump_begin = 0;
	UINT32 station_jump_end = 0;
	//������䳤��С��2000m������Ҫ��׶��Ż�
	if (interval_length < 2000)
	{
		lower_bound[0] = 0;
		upper_bound[0] = interval_length_cm;
		switch_flag[0] = 1;//ǣ��
		return 1;
	}
	//�����ն�׶��Ż�
	else
	{
		for (INT32 i = 0; i < limit_num; i++)
		{
			if (speed_limit[i] < 1500 && speed_limit_location[i]>50000)//�ҵ�վ̨���ٵ㣬������ͣ����Ϊ������
			{
				station_jump_begin = speed_limit_location[i - 1];
				station_jump_end = speed_limit_location[i];
				break;
			}
			else
			{
				continue;
			}
		}
		lower_bound[0] = 0;
		upper_bound[0] = station_jump_begin;
		switch_flag[0] = 1;//ǣ��-����

		lower_bound[1] = station_jump_end;
		upper_bound[1] = station_jump_end + (interval_length_cm - station_jump_end) / 3;
		switch_flag[1] = 2;//����-ǣ��

		lower_bound[2] = upper_bound[1];
		upper_bound[2] = interval_length_cm;
		switch_flag[2] = 1;//ǣ��-����
		return 3;
	}
}
