#pragma once

/*�����������Ͷ���*/
typedef char                CHAR;
typedef unsigned char       UCHAR;
typedef signed char         INT8;
typedef unsigned char       UINT8;
typedef short int           INT16;
typedef unsigned short int  UINT16;
typedef int                 INT32;
typedef unsigned int        UINT32;
typedef float               FLOAT32;
typedef double              FLOAT64;

/*�ٶȹ滮*/
#define SAFETY_THRESHOLD_EBI		138				// �Ƽ��ٶ���EBI����С��ֵ(5km/h)
#define GRAVITY_ACC					9.8				// �������ٶ� 9.8m/s^2
#define MAX_SPEED_CURVE				3000			// �Ż����ߵ����洢����
#define MAX_INTERVAL_SAMPLING		5000			// ������·���ݵ������������
#define MAX_LIMIT_POINT				30				// ������·�����л����������
#define SPEED_PLAN_TB_RATIO			0.6				// �ٶȹ滮ǣ���ƶ��������
#define MAX_AW_NUM					4				// �г��غ�ϸ��������AW0|AW1|AW2|AW3��
#define MAX_TBCURVE_NUM				200				// ����ǣ���ƶ�������������

/*λ�ýṹ��*/
typedef struct 
{
	UINT16 link_id;										/*Link id*/
	UINT32 link_offset;									/*Link offset*/
	UINT8  dir;											/*����*/
}TRAIN_LOACTION_STRU;

/*�г������ṹ��*/
typedef struct
{
	/*��������*/
	UINT32  train_length;                   /*�г�����, cm*/
	UINT16  init_v;                         /*��ʼ�ٶ�, cm/s*/
	UINT16  init_a;                         /*��ʼ���ٶ�, cm/s/s*/
	UINT16  aw[MAX_AW_NUM];                 /*�غ�, ��, aw[0-3]�ֱ�Ϊaw0 aw1 aw2(Ԥ��) aw3*/
	FLOAT64 a[MAX_AW_NUM];                  /*��ͬ�غ��µ�����ϵ��a*/
	FLOAT64 b[MAX_AW_NUM];                  /*��ͬ�غ��µ�����ϵ��b*/
	FLOAT64 c[MAX_AW_NUM];                  /*��ͬ�غ��µ�����ϵ��c*/
	/*ǣ���ƶ��������߲���*/
	UINT16  traction_num;                   /*ǣ����������ɢ������*/
	UINT16  traction_v[MAX_TBCURVE_NUM];    /*ǣ�����������ٶ��km/h*/
	UINT16  traction_a0[MAX_TBCURVE_NUM];   /*ǣ���������߼��ٶ���-AW0, cm/s/s*/
	UINT16  traction_a1[MAX_TBCURVE_NUM];   /*ǣ���������߼��ٶ���-AW1, cm/s/s*/
	UINT16  traction_a2[MAX_TBCURVE_NUM];   /*ǣ���������߼��ٶ���-AW2, cm/s/s*/
	UINT16  traction_a3[MAX_TBCURVE_NUM];   /*ǣ���������߼��ٶ���-AW3, cm/s/s*/
	UINT16  braking_num;                    /*�ƶ���������ɢ������*/
	UINT16  braking_v[MAX_TBCURVE_NUM];     /*�ƶ����������ٶ��km/h*/
	UINT16  braking_a0[MAX_TBCURVE_NUM];    /*�ƶ��������߼��ٶ���-AW0, cm/s/s*/
	UINT16  braking_a1[MAX_TBCURVE_NUM];    /*�ƶ��������߼��ٶ���-AW1, cm/s/s*/
	UINT16  braking_a2[MAX_TBCURVE_NUM];    /*�ƶ��������߼��ٶ���-AW2, cm/s/s*/
	UINT16  braking_a3[MAX_TBCURVE_NUM];    /*�ƶ��������߼��ٶ���-AW3, cm/s/s*/
	/*�����ƶ���*/
	UINT16  eb;                             /*�����ƶ��ʣ�cm/s/s*/
	/*��ʱ����*/
	FLOAT64 t_delay_10;                     /*ǣ��ʱ����ǣ��ָ������г����ٶȴﵽĿ����ٶȵ�10%����ʱ��s*/
	FLOAT64 t_delay_90;                     /*ǣ��ʱ����ǣ��ָ������г����ٶȴﵽĿ����ٶȵ�90%����ʱ��s*/
	FLOAT64 tr_delay_10;                    /*�г�ǣ��ʱ����ǣ���г�ָ��������г����ٶȽ���ԭ���ٶȵ�10%����ʱ��s*/
	FLOAT64 tr_delay_90;                    /*�г�ǣ��ʱ����ǣ���г�ָ��������г����ٶȽ���ԭ���ٶȵ�90%����ʱ��s*/
	FLOAT64 sb_delay_10;                    /*�����ƶ�ʱ���ӳ����ƶ�ָ��������г��ƶ��ʴﵽĿ���ƶ��ʵ�10%����ʱ��s*/
	FLOAT64 sb_delay_90;                    /*�����ƶ�ʱ���ӳ����ƶ�ָ��������г��ƶ��ʴﵽĿ���ƶ��ʵ�90%����ʱ��s*/
	FLOAT64 eb_delay_10;                    /*�����ƶ�ʱ���ӽ����ƶ�ָ��������г��ƶ��ʴﵽĿ���ƶ��ʵ�10%����ʱ��s*/
	FLOAT64 eb_delay_90;                    /*�����ƶ�ʱ���ӽ����ƶ�ָ��������г��ƶ��ʴﵽĿ���ƶ��ʵ�90%����ʱ��s*/
}TRAIN_PARAMETER;

/*��·���ݽṹ��*/
typedef struct
{
	UINT32 interval_length;					/*���䳤�� m*/
	UINT16 limit[MAX_INTERVAL_SAMPLING];	/*����1m��ɢ����·����, cm/s*/
	FLOAT32 gradient[MAX_INTERVAL_SAMPLING]; /*����1m��ɢ��·�¶ȣ���*/
	UINT32 curve_radius[MAX_INTERVAL_SAMPLING];/*����1m��ɢ��·���߰뾶*/
}LINE_PARAMETER;