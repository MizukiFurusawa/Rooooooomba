/****************************************************************************/
/* �Ώۃ}�C�R�� R8C/38A                                                     */
/* ̧�ٓ��e     �S�����|���@����v���O����									*/
/*              										                    */
/* Date         2015.01.22                                                  */
/****************************************************************************/





/*======================================*/
/* �C���N���[�h                         */
/*======================================*/
#include <stdio.h>
#include "sfr_r838a.h"                  /* R8C/38A SFR�̒�`�t�@�C��    */
#include "types3_beep.h"                /* �u�U�[�ǉ�                   */





/*======================================*/
/* �V���{����`                         */
/*======================================*/
#define     TRC_MOTOR_CYCLE     20000   /* ���O,�E�O���[�^PWM�̎���    50[ns] * 20000 = 1.00[ms]    */
#define     TRD_MOTOR_CYCLE     20000   /* ����,�E��,����Ӱ�PWM�̎���  50[ns] * 20000 = 1.00[ms]    */
#define     FREE                1       /* ���[�^���[�h�@�t���[         */
#define     BRAKE               0       /* ���[�^���[�h�@�u���[�L       */
#define 	MOVE_LEFT			0
#define 	MOVE_RIGHT			1
#define 	MOVE_STRAIGHT		2
#define 	MOVE_BACK			3
#define 	MOVE_BRAKE			4
#define 	TURN45				500
#define 	TURN90				1000
#define 	TURN120				1500
#define 	TURN180				2000
#define 	CHANGE_MODE_TIME	60000	//1�����ƂɃ��[�h�؂�ւ�
#define 	TOTAL_CLEANING_TIME	300000	//1��̑|���ɂ�5��������
#define 	TOUCH_TIME_MAX 		5000
#define 	TOUCH_CNT_MAX  		10
#define 	YUKA_BACK			300
#define 	PUSH_BACK			200





/*======================================*/
/* �v���g�^�C�v�錾                     */
/*======================================*/
void loop(void);
void init( void );
unsigned char dipsw_get( void );
unsigned char pushsw_get( void );
unsigned char cn6_get( void );
void led_out( unsigned char led );
void motor_r( int accele_l, int accele_r );
void motor_f( int accele_l, int accele_r );
void motor_mode_r( int mode_l, int mode_r );
void motor_mode_f( int mode_l, int mode_r );
void move_order(int direction,int get_move_time);
int get_sensor( void );




/*======================================*/
/* �O���[�o���ϐ��̐錾                 */
/*======================================*/
int             pattern;                /* �}�C�R���J�[����p�^�[��     */
unsigned long   cnt1;                   /* �^�C�}�p                     */
unsigned long   cnt2;                   /* �^�C�}�p                     */
unsigned long   cnt_cleaning_time;      /* �^�C�}�p                     */
unsigned long   cnt_cleaning_total_time;/* �^�C�}�p                     */
unsigned long   cnt_touch_time;      	/* �^�C�}�p                     */
int             iTimer10;               /* 10ms�J�E���g�p               */
long            lEncoderTotal;          /* �ώZ�l�ۑ��p                 */
int             iEncoder;               /* 10ms���̍ŐV�l               */
unsigned int    uEncoderBuff;           /* �v�Z�p�@���荞�ݓ��Ŏg�p     */
unsigned int    trcgrb_buff;            /* TRCGRB�̃o�b�t�@             */
unsigned int    trcgrd_buff;            /* TRCGRD�̃o�b�t�@             */
unsigned char   types_led;              /* LED�l�ݒ�                    */
unsigned char   types_dipsw;            /* �f�B�b�v�X�C�b�`�l�ۑ�       */
int				rotate_flag;
int				make_bit_led;
int				turn_buff[4];
int				now_turn;
int				cnt_touch = 0;
int				turn_flag = 0;
int				side_flag = 0;



/************************************************************************/
/* ���C���v���O����                                                     */
/************************************************************************/
void main( void ){
    int i;
	
    init();
    asm(" fset I ");
    initBeepS();
    motor_mode_f( BRAKE, BRAKE );
    motor_mode_r( BRAKE, BRAKE );
    motor_f( 0, 0 );
    motor_r( 0, 0 );
	rotate_flag = 0;
	cnt1 = 0;
	pattern = 0;
	now_turn = 0;
	turn_flag = 0;
	side_flag = 0;
	turn_buff[0] = TURN45;
	turn_buff[1] = TURN90;
	turn_buff[2] = TURN120;
	turn_buff[3] = TURN180;
	
	setBeepPatternS( 0x8000 );
    while( 1 )loop();
}





/************************************************************************/
/* ���C���v���O����                                                     */
/************************************************************************/
int get_sensor( void ){
	led_out( (~cn6_get())&0x0f );
	if( ((~cn6_get())&0x0f)!=0x0f )return 1; 
	if( p0_3 == 0 )return 2;
	return 0;
}




/************************************************************************/
/* ���C�����[�v		                                                    */
/************************************************************************/
void loop(void){
		
	if( cnt_cleaning_total_time > TOTAL_CLEANING_TIME )pattern = 1;
		
	switch( pattern ){
		
		
		
		//�f�o�b�O���[�h�`�F�b�N
		case 0:
		led_out(0xff);
		if( pushsw_get() == 1 )pattern = 3;
		else pattern = 1;
		while( pushsw_get()==1 );
		break;
		
		
		
		//�X�^�[�g�{�^���҂�
		case 1:
    	motor_r( 0, 0 );
    	motor_f( 0, 0 );
		if(cnt1 < 50)led_out(0xff);
		else if(cnt1 < 300)led_out(0x00);
		else cnt1 = 0;
		if( pushsw_get() == 1 ){
			cnt_touch = cnt_touch_time = 0;
			cnt_cleaning_total_time = 0;
			cnt1 = 0;
			cnt2 = 0;
			pattern = 2;
		}
		break;
		
		
		
		//2�b�҂�
		case 2:
		motor_r( 100 , 10 );	//�����@�z�����p�̃��[�^�A�E����]�u���V�̃��[�^
		if(cnt1 < 500)led_out(0xff);
		else if(cnt1 < 1000)led_out(0x00);
		else cnt1 = 0;
		if( cnt2 > 2000 ){
			led_out(0xff);
			cnt2 = cnt1 = 0;
			cnt_cleaning_time = 0;
			pattern = 100;
		}
		break;
		
		
		
		//�f�o�b�O���[�h�i�Z���T�A���񎞊ԃ`�F�b�N�j
		case 3:
		if( pushsw_get() == 1 )pattern = 4;
		if( p0_3 == 0 )make_bit_led = 0xf0;
		else make_bit_led = 0x00;
		
		make_bit_led |= (~cn6_get())&0x0f;
		led_out( make_bit_led & 0xff );
		break;
		
		
		
		case 4:
		move_order( MOVE_RIGHT , TURN45 );
		while( pushsw_get()==1 );
		pattern = 3;
		break;
		
		
		
		
		
		
		
		
		
		case 100:
		if( pushsw_get() == 1 ){
			pattern = 1;
			while( pushsw_get() );
			break;	
		}
    	motor_f( 100, 100 );
		if( cnt1 > 3000 ){	//3�b�Ԃ͒��i
			pattern = 200;
			cnt_cleaning_time = 0;
			cnt1 = 0;
		}
		if( get_sensor() == 1 ){	//���Z���T������������A
			motor_f( -100, -100 );
			while( ((~cn6_get())&0x0f)!=0x0f );	//�������Ȃ��Ȃ�܂Ńo�b�N
			pattern = 200;
			cnt_cleaning_time = 0;
			cnt1 = 0;
		}else if( get_sensor() == 2 ){	//�ڐG�Z���T������������A
			move_order(MOVE_BACK,PUSH_BACK);	//PUSH_BACK�����o�b�N
			pattern = 200;
			cnt_cleaning_time = 0;
			cnt1 = 0;
		}
		break;
		
		
		
		
		
		
		
		
		
		
		//���邮����񂳂���
		case 200:
		if( pushsw_get() == 1 ){
			pattern = 1;
			while( pushsw_get() );
			break;
		}
		if( cnt_cleaning_time > CHANGE_MODE_TIME ){	//��莞�Ԍo�ߌ�ɁA���̑|�����[�h��
			cnt_cleaning_time = 0;
			pattern = 300;
			break;
		}
		if( get_sensor() != 0  || cnt1/1200 > 100 ){	//���̃��[�h���ɃZ���T�������������莞�Ԍo�߂����玟�̑|�����[�h��
			motor_f( -100 , -100 );
			while( ((~cn6_get())&0x0f)!=0x0f );
			move_order( MOVE_BACK , YUKA_BACK );
			cnt_cleaning_time = cnt_touch_time = 0;
			pattern = 300;
			break;
		}
		motor_f( cnt1/1200 , 100 );	
		break;
		
		
		
		
		
		
		
		
		
		
		//�ʏ퓮��
		case 300:
		if( pushsw_get() == 1 ){
			pattern = 1;
			while( pushsw_get() );
			break;
		}
		if( cnt_cleaning_time > CHANGE_MODE_TIME ){	//��莞�Ԍo�ߌ�ɁA���̑|�����[�h��
			cnt_cleaning_time = 0;
			cnt1 = 0;
			if( side_flag == 0 )pattern = 400;
			if( side_flag == 1 )pattern = 500;
			side_flag = 1 - side_flag;	//���̑|�����[�h�́A����|�����܂����Aside_flag��0�ō����̋����A1�ŉE���̋���|�����܂�
			break;	
		}
		
		
		// turn_flag ��0�̎��͉E��]�A1�̎��͍���]
		// ��]���Ԃ�4��ނ����āA�Z���T���ǂꂩ1�񔽉����邲�ƂɎ��̉�]���Ԃ��Z�b�g
		// 4��Z���T������������@��]���Ԃ��ŏ��ɖ߂��ā@����4��͋t��]������
		// turn_flag ����]�����Ȃ̂Ł@turn_flag = 1 - turn_flag;�@��0��1�����ւ���
		// get_sensor()�́@�Ȃɂ��������Ȃ�������0�A���Z���T������1�A�ڐG�Z���T������2��Ԃ��܂�
		
		if( get_sensor() == 1 ){
			motor_f( -100, -100 );
			while( ((~cn6_get())&0x0f)!=0x0f );
			move_order( MOVE_BACK , YUKA_BACK );
			if( turn_flag == 0 )move_order( MOVE_RIGHT , turn_buff[now_turn] );
			if( turn_flag == 1 )move_order( MOVE_LEFT , turn_buff[now_turn] );
			if( now_turn >= 3 ){turn_flag = 1 - turn_flag ;now_turn = 0;}
			else now_turn++;
			break;
		}else if( get_sensor() == 2 ){
			move_order( MOVE_BACK , PUSH_BACK );
			if( turn_flag == 0 )move_order( MOVE_RIGHT , turn_buff[now_turn] );
			if( turn_flag == 1 )move_order( MOVE_LEFT , turn_buff[now_turn] );
			if( now_turn >= 3 ){turn_flag = 1 - turn_flag ;now_turn = 0;}
			else now_turn++;
			break;
		}
    	motor_f( 100, 100 );
		break;
		
		
		
		
		
		
		
		
		
		
		//�����d�_�I��
		case 400:
		if( pushsw_get() == 1 ){
			pattern = 1;
			while( pushsw_get() );
			break;	
		}
		if( cnt_cleaning_time > CHANGE_MODE_TIME ){
			cnt_cleaning_time = 0;
			pattern = 300;
			break;	
		}
		if( get_sensor() == 1 ){
			cnt1 = 0;
			motor_f( -100, -100 );
			while( ((~cn6_get())&0x0f)!=0x0f );
			move_order( MOVE_BACK , PUSH_BACK );
			move_order( MOVE_RIGHT , TURN45/2 );
			break;
		}else if( get_sensor() == 2 ){
			cnt1 = 0;
			move_order( MOVE_BACK , PUSH_BACK );
			move_order( MOVE_RIGHT , TURN45/2 );
			break;
		}
		if( cnt1 < 5000	)motor_f(60, 100 );
		else if( cnt1 >= 5000 && cnt1 < 10000)motor_f( 100, 100 );
		else cnt1 = 0;
    	
		break;
		
		
		
		
		
		
		
		
		
		
		//�����d�_�I��
		case 500:
		if( pushsw_get() == 1 ){
			pattern = 1;
			while( pushsw_get() );
			break;	
		}
		if( cnt_cleaning_time > CHANGE_MODE_TIME ){
			cnt_cleaning_time = 0;
			pattern = 300;
			break;	
		}
		if( get_sensor() == 1 ){
			cnt1 = 0;
			motor_f( -100, -100 );
			while( ((~cn6_get())&0x0f)!=0x0f );
			move_order( MOVE_BACK , PUSH_BACK );
			move_order( MOVE_LEFT , TURN45/2 );
			break;
		}else if( get_sensor() == 2 ){
			cnt1 = 0;
			move_order( MOVE_BACK , PUSH_BACK );
			move_order( MOVE_LEFT , TURN45/2 );
			break;
		}
		if( cnt1 < 5000	)motor_f(100, 60 );
		else if( cnt1 >= 5000 && cnt1 < 10000)motor_f( 100, 100 );
		else cnt1 = 0;
    	
		break;
		
		
		case 1000:
  	 	motor_r( 0, 0 );
  	 	motor_f( 0, 0 );
		break;
	}
	
}





/************************************************************************/
/* R8C/38A �X�y�V�����t�@���N�V�������W�X�^(SFR)�̏�����                */
/************************************************************************/
void init( void )
{
    int     i;

    /* �N���b�N��XIN�N���b�N(20MHz)�ɕύX */
    prc0  = 1;                          /* �v���e�N�g����               */
    cm13  = 1;                          /* P4_6,P4_7��XIN-XOUT�[�q�ɂ���*/
    cm05  = 0;                          /* XIN�N���b�N���U              */
    for(i=0; i<50; i++ );               /* ���肷��܂ŏ����҂�(��10ms) */
    ocd2  = 0;                          /* �V�X�e���N���b�N��XIN�ɂ���  */
    prc0  = 0;                          /* �v���e�N�gON                 */

    /* �|�[�g�̓��o�͐ݒ� */

    /*  PWM(�\��)       ���OM_PMW       �E�OM_PWM       �u�U�[
        �Z���T���[      �Z���T����      �Z���T�E��      �Z���T�E�[  */
    p0   = 0x00;
    prc2 = 1;                           /* PD0�̃v���e�N�g����          */
    pd0  = 0xf0;

    /*  �Z���T���S      �����ް         RxD0            TxD0
        DIPSW3          DIPSW2          DIPSW1          DIPSW0          */
    pur0 |= 0x04;                       /* P1_3�`P1_0�̃v���A�b�vON     */
    p1  = 0x00;
    pd1 = 0x10;

    /*  �E�OM_����      �X�e�AM_����    �X�e�AM_PWM     �E��M_PWM
        �E��M_����      ����M_PWM       ����M_����      ���OM_����      */
    p2  = 0x00;
    pd2 = 0xff;

    /*  none            none            none            none
        none            none            none            �G���R�[�_A��   */
    p3  = 0x00;
    pd3 = 0xfe;

    /*  XOUT            XIN             �{�[�h���LED   none
        none            VREF            none            none            */
    p4  = 0x20;                         /* P4_5��LED:�����͓_��         */
    pd4 = 0xb8;

    /*  none            none            none            none
        none            none            none            none            */
    p5  = 0x00;
    pd5 = 0xff;

    /*  none            none            none            none
        none            none            none            none            */
    p6  = 0x00;
    pd6 = 0xff;

    /*  CN6.2����       CN6.3����       CN6.4����       CN6.5����
        none(��۸ޗ\��) �p�xVR          �Z���T_����۸�  �Z���T_�E��۸�  */
    p7  = 0x00;
    pd7 = 0x00;

    /*  DIPSWorLED      DIPSWorLED      DIPSWorLED      DIPSWorLED
        DIPSWorLED      DIPSWorLED      DIPSWorLED      DIPSWorLED      */
    pur2 |= 0x03;                       /* P8_7�`P8_0�̃v���A�b�vON     */
    p8  = 0x00;
    pd8 = 0x00;

    /*  -               -               �߯������       P8����(LEDorSW)
        �E�OM_Free      ���OM_Free      �E��M_Free      ����M_Free      */
    p9  = 0x00;
    pd9 = 0x1f;
    pu23 = 1;   // P9_4,P9_5���v���A�b�v����
    /* �^�C�}RB�̐ݒ� */
    /* ���荞�ݎ��� = 1 / 20[MHz]   * (TRBPRE+1) * (TRBPR+1)
                    = 1 / (20*10^6) * 200        * 100
                    = 0.001[s] = 1[ms]
    */
    trbmr  = 0x00;                      /* ���샂�[�h�A������ݒ�       */
    trbpre = 200-1;                     /* �v���X�P�[�����W�X�^         */
    trbpr  = 100-1;                     /* �v���C�}�����W�X�^           */
    trbic  = 0x06;                      /* ���荞�ݗD�惌�x���ݒ�       */
    trbcr  = 0x01;                      /* �J�E���g�J�n                 */

    /* A/D�R���o�[�^�̐ݒ� */
    admod   = 0x33;                     /* �J��Ԃ��|�����[�h�ɐݒ�     */
    adinsel = 0x90;                     /* ���͒[�qP7��4�[�q��I��      */
    adcon1  = 0x30;                     /* A/D����\                  */
    asm(" nop ");                       /* ��AD��1�T�C�N���E�G�C�g�����*/
    adcon0  = 0x01;                     /* A/D�ϊ��X�^�[�g              */

    /* �^�C�}RG �^�C�}���[�h(���G�b�W�ŃJ�E���g)�̐ݒ� */
    timsr = 0x40;                       /* TRGCLKA�[�q P3_0�Ɋ��蓖�Ă� */
    trgcr = 0x15;                       /* TRGCLKA�[�q�̗��G�b�W�ŃJ�E���g*/
    trgmr = 0x80;                       /* TRG�̃J�E���g�J�n            */

    /* �^�C�}RC PWM���[�h�ݒ�(���O���[�^�A�E�O���[�^) */
    trcpsr0 = 0x40;                     /* TRCIOA,B�[�q�̐ݒ�           */
    trcpsr1 = 0x33;                     /* TRCIOC,D�[�q�̐ݒ�           */
    trcmr   = 0x0f;                     /* PWM���[�h�I���r�b�g�ݒ�      */
    trccr1  = 0x8e;                     /* �������:f1,�����o�͂̐ݒ�    */
    trccr2  = 0x00;                     /* �o�̓��x���̐ݒ�             */
    trcgra  = TRC_MOTOR_CYCLE - 1;      /* �����ݒ�                     */
    trcgrb  = trcgrb_buff = trcgra;     /* P0_5�[�q��ON��(���O���[�^)   */
    trcgrc  = trcgra;                   /* P0_7�[�q��ON��(�\��)         */
    trcgrd  = trcgrd_buff = trcgra;     /* P0_6�[�q��ON��(�E�O���[�^)   */
    trcic   = 0x07;                     /* ���荞�ݗD�惌�x���ݒ�       */
    trcier  = 0x01;                     /* IMIA������                   */
    trcoer  = 0x01;                     /* �o�͒[�q�̑I��               */
    trcmr  |= 0x80;                     /* TRC�J�E���g�J�n              */

    /* �^�C�}RD ���Z�b�g����PWM���[�h�ݒ�(����Ӱ��A�E��Ӱ��A����Ӱ�) */
    trdpsr0 = 0x08;                     /* TRDIOB0,C0,D0�[�q�ݒ�        */
    trdpsr1 = 0x05;                     /* TRDIOA1,B1,C1,D1�[�q�ݒ�     */
    trdmr   = 0xf0;                     /* �o�b�t�@���W�X�^�ݒ�         */
    trdfcr  = 0x01;                     /* ���Z�b�g����PWM���[�h�ɐݒ�  */
    trdcr0  = 0x20;                     /* �\�[�X�J�E���g�̑I��:f1      */
    trdgra0 = trdgrc0 = TRD_MOTOR_CYCLE - 1;    /* �����ݒ�             */
    trdgrb0 = trdgrd0 = 0;              /* P2_2�[�q��ON��(���ヂ�[�^)   */
    trdgra1 = trdgrc1 = 0;              /* P2_4�[�q��ON��(�E�ヂ�[�^)   */
    trdgrb1 = trdgrd1 = 0;              /* P2_5�[�q��ON��(�T�[�{���[�^) */
    trdoer1 = 0xcd;                     /* �o�͒[�q�̑I��               */
    trdstr  = 0x0d;                     /* TRD0�J�E���g�J�n             */
}





/************************************************************************/
/* �^�C�}RC ���荞�ݏ���                                                */
/************************************************************************/
#pragma interrupt intTRC(vect=7)
void intTRC( void ){
    trcsr &= 0xfe;

    /* �^�C�}RC�@�f���[�e�B��̐ݒ� */
    trcgrb = trcgrb_buff;
    trcgrd = trcgrd_buff;
}





/************************************************************************/
/* �}�C�R���{�[�h��̃f�B�b�v�X�C�b�`�l�ǂݍ���                         */
/* �����@ �Ȃ�                                                          */
/* �߂�l �X�C�b�`�l 0�`15                                              */
/************************************************************************/
unsigned char dipsw_get( void ){
    unsigned char sw;
    sw = p1 & 0x0f;                     /* P1_3�`P1_0�ǂݍ���           */
    return sw;
}





/************************************************************************/
/* ���[�^�h���C�u���TypeS Ver.3��̃v�b�V���X�C�b�`�l�ǂݍ���          */
/* �����@ �Ȃ�                                                          */
/* �߂�l �X�C�b�`�l 0:OFF 1:ON                                         */
/************************************************************************/
unsigned char pushsw_get( void ){
    unsigned char sw;
    sw = ~p9_5 & 0x01;
    return sw;
}





/************************************************************************/
/* ���[�^�h���C�u���TypeS Ver.3��CN6�̏�ԓǂݍ���                     */
/* �����@ �Ȃ�                                                          */
/* �߂�l 0�`15                                                         */
/************************************************************************/
unsigned char cn6_get( void ){
    unsigned char data;
    data = p7 >> 4;
    return data;
}





/************************************************************************/
/* ���[�^�h���C�u���TypeS Ver.3��LED����                               */
/* �����@ 8��LED���� 0:OFF 1:ON                                       */
/* �߂�l �Ȃ�                                                          */
/************************************************************************/
void led_out( unsigned char led )
{
    /* ���ۂ̏o�͂̓^�C�}RB���荞�ݏ����Ŏ��{ */
    types_led = led;
}





/************************************************************************/
/* ��ւ̑��x����                                                       */
/* �����@ �����[�^:-100�`100 , �E���[�^:-100�`100                       */
/*        0�Œ�~�A100�Ő��]100%�A-100�ŋt�]100%                        */
/* �߂�l �Ȃ�                                                          */
/************************************************************************/
void motor_r( int accele_l, int accele_r )
{
    int sw_data;

    sw_data  = 20;         /* �f�B�b�v�X�C�b�`�ǂݍ���     */
    accele_l = accele_l * sw_data / 20;
    accele_r = accele_r * sw_data / 20;

    /* ���ヂ�[�^ */
    if( accele_l >= 0 ) {
        p2_1 = 0;
        trdgrd0 = (long)( TRD_MOTOR_CYCLE - 2 ) * accele_l / 100;
    } else {
        p2_1 = 1;
        trdgrd0 = (long)( TRD_MOTOR_CYCLE - 2 ) * ( -accele_l ) / 100;
    }

    /* �E�ヂ�[�^ */
    if( accele_r >= 0 ) {
        p2_3 = 0;
        trdgrc1 = (long)( TRD_MOTOR_CYCLE - 2 ) * accele_r / 100;
    } else {
        p2_3 = 1;
        trdgrc1 = (long)( TRD_MOTOR_CYCLE - 2 ) * ( -accele_r ) / 100;
    }
}





/************************************************************************/
/* �O�ւ̑��x����                                                       */
/* �����@ �����[�^:-100�`100 , �E���[�^:-100�`100                       */
/*        0�Œ�~�A100�Ő��]100%�A-100�ŋt�]100%                        */
/* �߂�l �Ȃ�                                                          */
/************************************************************************/
void motor_f( int accele_l, int accele_r )
{
    int sw_data;

    sw_data  = dipsw_get() + 5;         /* �f�B�b�v�X�C�b�`�ǂݍ���     */
    accele_l = accele_l * sw_data / 20;
    accele_r = accele_r * sw_data / 20;

    /* ���O���[�^ */
    if( accele_l >= 0 ) {
        p2_0 = 0;
    } else {
        p2_0 = 1;
        accele_l = -accele_l;
    }
    if( accele_l <= 5 ) {
        trcgrb = trcgrb_buff = trcgra;
    } else {
        trcgrb_buff = (unsigned long)(TRC_MOTOR_CYCLE-2) * accele_l / 100;
    }

    /* �E�O���[�^ */
    if( accele_r >= 0 ) {
        p2_7 = 0;
    } else {
        p2_7 = 1;
        accele_r = -accele_r;
    }
    if( accele_r <= 5 ) {
        trcgrd = trcgrd_buff = trcgra;
    } else {
        trcgrd_buff = (unsigned long)(TRC_MOTOR_CYCLE-2) * accele_r / 100;
    }
}





/************************************************************************/
/* �ヂ�[�^��~����i�u���[�L�A�t���[�j                                 */
/* �����@ �����[�^:FREE or BRAKE , �E���[�^:FREE or BRAKE               */
/* �߂�l �Ȃ�                                                          */
/************************************************************************/
void motor_mode_r( int mode_l, int mode_r )
{
    if( mode_l ) {
        p9_0 = 1;
    } else {
        p9_0 = 0;
    }
    if( mode_r ) {
        p9_1 = 1;
    } else {
        p9_1 = 0;
    }
}





/************************************************************************/
/* �O���[�^��~����i�u���[�L�A�t���[�j                                 */
/* �����@ �����[�^:FREE or BRAKE , �E���[�^:FREE or BRAKE               */
/* �߂�l �Ȃ�                                                          */
/************************************************************************/
void motor_mode_f( int mode_l, int mode_r )
{
    if( mode_l ) {
        p9_2 = 1;
    } else {
        p9_2 = 0;
    }
    if( mode_r ) {
        p9_3 = 1;
    } else {
        p9_3 = 0;
    }
}





/************************************************************************/
/* �^�C�}RB ���荞�ݏ���                                                */
/************************************************************************/
#pragma interrupt /B intTRB(vect=24)
void intTRB( void )
{
    unsigned int i;

    asm(" fset I ");                    /* �^�C�}RB�ȏ�̊��荞�݋���   */

    cnt1++;
	cnt2++;
	cnt_cleaning_time++;
	cnt_cleaning_total_time++;
	cnt_touch_time++;
	
    /* �u�U�[���� */
    beepProcessS();

    /* 10��1����s���鏈�� */
    iTimer10++;
    switch( iTimer10 ) {
    case 1:
        /* �G���R�[�_���� */
        i = trg;
        iEncoder       = i - uEncoderBuff;
        lEncoderTotal += iEncoder;
        uEncoderBuff = i;
        break;

    case 2:
        /* �X�C�b�`�ǂݍ��ݏ��� */
        p9_4 = 0;                       /* LED�o��OFF                   */
        pd8  = 0x00;
        break;

    case 3:
        /* �X�C�b�`�ǂݍ��݁ALED�o�� */
        types_dipsw = ~p8;              /* ��ײ�ފ��TypeS Ver.3��SW�ǂݍ���*/
        p8  = types_led;                /* ��ײ�ފ��TypeS Ver.3��LED�֏o��*/
        pd8 = 0xff;
        p9_4 = 1;                       /* LED�o��ON                    */
        break;

    case 4:
        break;

    case 5:
        break;

    case 6:
        break;

    case 7:
        break;

    case 8:
        break;

    case 9:
        break;

    case 10:
        /* iTimer10�ϐ��̏��� */
        iTimer10 = 0;
        break;
    }
}





/************************************************************************/
/* �w�莞�Ԉړ�������	                                                */
/************************************************************************/
void move_order(int direction,int get_move_time){
	
	switch( direction ){
		
		//����]
		case MOVE_LEFT:
		motor_f(-100,100);
		break;
		
		//�E��]
		case MOVE_RIGHT:
		motor_f(100,-100);
		break;
		
		//���i
		case MOVE_STRAIGHT:
		motor_f(100,100);
		break;
		
		//��i
		case MOVE_BACK:
		motor_f(-100,-100);
		break;
		
		default:
		return;
		
	}
	cnt2 = 0;
	
	//�w�莞�ԕ�������p��
	while( cnt2 < get_move_time ){
		if( pushsw_get() == 1 )break;	
	}
	motor_f(0,0);
}





/************************************************************************/
/* end of file                                                          */
/************************************************************************/

/*
�����o��

2011.06.01 Ver.1.00 �쐬
2012.02.23 Ver.1.01 ���[�^�h���C�u���TypeS Ver.3��
                    �A�i���O�Z���T���TypeS Ver.2�̃R�����g�ύX
2013.05.10 Ver.2.00 ���[�^�h���C�u���TypeS Ver.4�ɑΉ�
*/