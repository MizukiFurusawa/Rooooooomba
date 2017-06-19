/****************************************************************************/
/* 対象マイコン R8C/38A                                                     */
/* ﾌｧｲﾙ内容     全自動掃除機制御プログラム									*/
/*              										                    */
/* Date         2015.01.22                                                  */
/****************************************************************************/





/*======================================*/
/* インクルード                         */
/*======================================*/
#include <stdio.h>
#include "sfr_r838a.h"                  /* R8C/38A SFRの定義ファイル    */
#include "types3_beep.h"                /* ブザー追加                   */





/*======================================*/
/* シンボル定義                         */
/*======================================*/
#define     TRC_MOTOR_CYCLE     20000   /* 左前,右前モータPWMの周期    50[ns] * 20000 = 1.00[ms]    */
#define     TRD_MOTOR_CYCLE     20000   /* 左後,右後,ｻｰﾎﾞﾓｰﾀPWMの周期  50[ns] * 20000 = 1.00[ms]    */
#define     FREE                1       /* モータモード　フリー         */
#define     BRAKE               0       /* モータモード　ブレーキ       */
#define 	MOVE_LEFT			0
#define 	MOVE_RIGHT			1
#define 	MOVE_STRAIGHT		2
#define 	MOVE_BACK			3
#define 	MOVE_BRAKE			4
#define 	TURN45				500
#define 	TURN90				1000
#define 	TURN120				1500
#define 	TURN180				2000
#define 	CHANGE_MODE_TIME	60000	//1分ごとにモード切り替え
#define 	TOTAL_CLEANING_TIME	300000	//1回の掃除につき5分かける
#define 	TOUCH_TIME_MAX 		5000
#define 	TOUCH_CNT_MAX  		10
#define 	YUKA_BACK			300
#define 	PUSH_BACK			200





/*======================================*/
/* プロトタイプ宣言                     */
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
/* グローバル変数の宣言                 */
/*======================================*/
int             pattern;                /* マイコンカー動作パターン     */
unsigned long   cnt1;                   /* タイマ用                     */
unsigned long   cnt2;                   /* タイマ用                     */
unsigned long   cnt_cleaning_time;      /* タイマ用                     */
unsigned long   cnt_cleaning_total_time;/* タイマ用                     */
unsigned long   cnt_touch_time;      	/* タイマ用                     */
int             iTimer10;               /* 10msカウント用               */
long            lEncoderTotal;          /* 積算値保存用                 */
int             iEncoder;               /* 10ms毎の最新値               */
unsigned int    uEncoderBuff;           /* 計算用　割り込み内で使用     */
unsigned int    trcgrb_buff;            /* TRCGRBのバッファ             */
unsigned int    trcgrd_buff;            /* TRCGRDのバッファ             */
unsigned char   types_led;              /* LED値設定                    */
unsigned char   types_dipsw;            /* ディップスイッチ値保存       */
int				rotate_flag;
int				make_bit_led;
int				turn_buff[4];
int				now_turn;
int				cnt_touch = 0;
int				turn_flag = 0;
int				side_flag = 0;



/************************************************************************/
/* メインプログラム                                                     */
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
/* メインプログラム                                                     */
/************************************************************************/
int get_sensor( void ){
	led_out( (~cn6_get())&0x0f );
	if( ((~cn6_get())&0x0f)!=0x0f )return 1; 
	if( p0_3 == 0 )return 2;
	return 0;
}




/************************************************************************/
/* メインループ		                                                    */
/************************************************************************/
void loop(void){
		
	if( cnt_cleaning_total_time > TOTAL_CLEANING_TIME )pattern = 1;
		
	switch( pattern ){
		
		
		
		//デバッグモードチェック
		case 0:
		led_out(0xff);
		if( pushsw_get() == 1 )pattern = 3;
		else pattern = 1;
		while( pushsw_get()==1 );
		break;
		
		
		
		//スタートボタン待ち
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
		
		
		
		//2秒待ち
		case 2:
		motor_r( 100 , 10 );	//左が　吸い取り用のモータ、右が回転ブラシのモータ
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
		
		
		
		//デバッグモード（センサ、旋回時間チェック）
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
		if( cnt1 > 3000 ){	//3秒間は直進
			pattern = 200;
			cnt_cleaning_time = 0;
			cnt1 = 0;
		}
		if( get_sensor() == 1 ){	//床センサが反応したら、
			motor_f( -100, -100 );
			while( ((~cn6_get())&0x0f)!=0x0f );	//反応しなくなるまでバック
			pattern = 200;
			cnt_cleaning_time = 0;
			cnt1 = 0;
		}else if( get_sensor() == 2 ){	//接触センサが反応したら、
			move_order(MOVE_BACK,PUSH_BACK);	//PUSH_BACKだけバック
			pattern = 200;
			cnt_cleaning_time = 0;
			cnt1 = 0;
		}
		break;
		
		
		
		
		
		
		
		
		
		
		//ぐるぐる旋回させる
		case 200:
		if( pushsw_get() == 1 ){
			pattern = 1;
			while( pushsw_get() );
			break;
		}
		if( cnt_cleaning_time > CHANGE_MODE_TIME ){	//一定時間経過後に、次の掃除モードへ
			cnt_cleaning_time = 0;
			pattern = 300;
			break;
		}
		if( get_sensor() != 0  || cnt1/1200 > 100 ){	//このモード中にセンサが反応したり一定時間経過したら次の掃除モードへ
			motor_f( -100 , -100 );
			while( ((~cn6_get())&0x0f)!=0x0f );
			move_order( MOVE_BACK , YUKA_BACK );
			cnt_cleaning_time = cnt_touch_time = 0;
			pattern = 300;
			break;
		}
		motor_f( cnt1/1200 , 100 );	
		break;
		
		
		
		
		
		
		
		
		
		
		//通常動作
		case 300:
		if( pushsw_get() == 1 ){
			pattern = 1;
			while( pushsw_get() );
			break;
		}
		if( cnt_cleaning_time > CHANGE_MODE_TIME ){	//一定時間経過後に、次の掃除モードへ
			cnt_cleaning_time = 0;
			cnt1 = 0;
			if( side_flag == 0 )pattern = 400;
			if( side_flag == 1 )pattern = 500;
			side_flag = 1 - side_flag;	//次の掃除モードは、隅を掃除しますが、side_flagが0で左側の隅を、1で右側の隅を掃除します
			break;	
		}
		
		
		// turn_flag が0の時は右回転、1の時は左回転
		// 回転時間は4種類あって、センサがどれか1回反応するごとに次の回転時間をセット
		// 4回センサが反応したら　回転時間を最初に戻して　次の4回は逆回転させる
		// turn_flag が回転方向なので　turn_flag = 1 - turn_flag;　で0と1を入れ替える
		// get_sensor()は　なにも反応しなかったら0、床センサ反応で1、接触センサ反応で2を返します
		
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
		
		
		
		
		
		
		
		
		
		
		//隅を重点的に
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
		
		
		
		
		
		
		
		
		
		
		//隅を重点的に
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
/* R8C/38A スペシャルファンクションレジスタ(SFR)の初期化                */
/************************************************************************/
void init( void )
{
    int     i;

    /* クロックをXINクロック(20MHz)に変更 */
    prc0  = 1;                          /* プロテクト解除               */
    cm13  = 1;                          /* P4_6,P4_7をXIN-XOUT端子にする*/
    cm05  = 0;                          /* XINクロック発振              */
    for(i=0; i<50; i++ );               /* 安定するまで少し待つ(約10ms) */
    ocd2  = 0;                          /* システムクロックをXINにする  */
    prc0  = 0;                          /* プロテクトON                 */

    /* ポートの入出力設定 */

    /*  PWM(予備)       左前M_PMW       右前M_PWM       ブザー
        センサ左端      センサ左中      センサ右中      センサ右端  */
    p0   = 0x00;
    prc2 = 1;                           /* PD0のプロテクト解除          */
    pd0  = 0xf0;

    /*  センサ中心      ｽﾀｰﾄﾊﾞｰ         RxD0            TxD0
        DIPSW3          DIPSW2          DIPSW1          DIPSW0          */
    pur0 |= 0x04;                       /* P1_3～P1_0のプルアップON     */
    p1  = 0x00;
    pd1 = 0x10;

    /*  右前M_方向      ステアM_方向    ステアM_PWM     右後M_PWM
        右後M_方向      左後M_PWM       左後M_方向      左前M_方向      */
    p2  = 0x00;
    pd2 = 0xff;

    /*  none            none            none            none
        none            none            none            エンコーダA相   */
    p3  = 0x00;
    pd3 = 0xfe;

    /*  XOUT            XIN             ボード上のLED   none
        none            VREF            none            none            */
    p4  = 0x20;                         /* P4_5のLED:初期は点灯         */
    pd4 = 0xb8;

    /*  none            none            none            none
        none            none            none            none            */
    p5  = 0x00;
    pd5 = 0xff;

    /*  none            none            none            none
        none            none            none            none            */
    p6  = 0x00;
    pd6 = 0xff;

    /*  CN6.2入力       CN6.3入力       CN6.4入力       CN6.5入力
        none(ｱﾅﾛｸﾞ予備) 角度VR          センサ_左ｱﾅﾛｸﾞ  センサ_右ｱﾅﾛｸﾞ  */
    p7  = 0x00;
    pd7 = 0x00;

    /*  DIPSWorLED      DIPSWorLED      DIPSWorLED      DIPSWorLED
        DIPSWorLED      DIPSWorLED      DIPSWorLED      DIPSWorLED      */
    pur2 |= 0x03;                       /* P8_7～P8_0のプルアップON     */
    p8  = 0x00;
    pd8 = 0x00;

    /*  -               -               ﾌﾟｯｼｭｽｲｯﾁ       P8制御(LEDorSW)
        右前M_Free      左前M_Free      右後M_Free      左後M_Free      */
    p9  = 0x00;
    pd9 = 0x1f;
    pu23 = 1;   // P9_4,P9_5をプルアップする
    /* タイマRBの設定 */
    /* 割り込み周期 = 1 / 20[MHz]   * (TRBPRE+1) * (TRBPR+1)
                    = 1 / (20*10^6) * 200        * 100
                    = 0.001[s] = 1[ms]
    */
    trbmr  = 0x00;                      /* 動作モード、分周比設定       */
    trbpre = 200-1;                     /* プリスケーラレジスタ         */
    trbpr  = 100-1;                     /* プライマリレジスタ           */
    trbic  = 0x06;                      /* 割り込み優先レベル設定       */
    trbcr  = 0x01;                      /* カウント開始                 */

    /* A/Dコンバータの設定 */
    admod   = 0x33;                     /* 繰り返し掃引モードに設定     */
    adinsel = 0x90;                     /* 入力端子P7の4端子を選択      */
    adcon1  = 0x30;                     /* A/D動作可能                  */
    asm(" nop ");                       /* φADの1サイクルウエイト入れる*/
    adcon0  = 0x01;                     /* A/D変換スタート              */

    /* タイマRG タイマモード(両エッジでカウント)の設定 */
    timsr = 0x40;                       /* TRGCLKA端子 P3_0に割り当てる */
    trgcr = 0x15;                       /* TRGCLKA端子の両エッジでカウント*/
    trgmr = 0x80;                       /* TRGのカウント開始            */

    /* タイマRC PWMモード設定(左前モータ、右前モータ) */
    trcpsr0 = 0x40;                     /* TRCIOA,B端子の設定           */
    trcpsr1 = 0x33;                     /* TRCIOC,D端子の設定           */
    trcmr   = 0x0f;                     /* PWMモード選択ビット設定      */
    trccr1  = 0x8e;                     /* ｿｰｽｶｳﾝﾄ:f1,初期出力の設定    */
    trccr2  = 0x00;                     /* 出力レベルの設定             */
    trcgra  = TRC_MOTOR_CYCLE - 1;      /* 周期設定                     */
    trcgrb  = trcgrb_buff = trcgra;     /* P0_5端子のON幅(左前モータ)   */
    trcgrc  = trcgra;                   /* P0_7端子のON幅(予備)         */
    trcgrd  = trcgrd_buff = trcgra;     /* P0_6端子のON幅(右前モータ)   */
    trcic   = 0x07;                     /* 割り込み優先レベル設定       */
    trcier  = 0x01;                     /* IMIAを許可                   */
    trcoer  = 0x01;                     /* 出力端子の選択               */
    trcmr  |= 0x80;                     /* TRCカウント開始              */

    /* タイマRD リセット同期PWMモード設定(左後ﾓｰﾀ、右後ﾓｰﾀ、ｻｰﾎﾞﾓｰﾀ) */
    trdpsr0 = 0x08;                     /* TRDIOB0,C0,D0端子設定        */
    trdpsr1 = 0x05;                     /* TRDIOA1,B1,C1,D1端子設定     */
    trdmr   = 0xf0;                     /* バッファレジスタ設定         */
    trdfcr  = 0x01;                     /* リセット同期PWMモードに設定  */
    trdcr0  = 0x20;                     /* ソースカウントの選択:f1      */
    trdgra0 = trdgrc0 = TRD_MOTOR_CYCLE - 1;    /* 周期設定             */
    trdgrb0 = trdgrd0 = 0;              /* P2_2端子のON幅(左後モータ)   */
    trdgra1 = trdgrc1 = 0;              /* P2_4端子のON幅(右後モータ)   */
    trdgrb1 = trdgrd1 = 0;              /* P2_5端子のON幅(サーボモータ) */
    trdoer1 = 0xcd;                     /* 出力端子の選択               */
    trdstr  = 0x0d;                     /* TRD0カウント開始             */
}





/************************************************************************/
/* タイマRC 割り込み処理                                                */
/************************************************************************/
#pragma interrupt intTRC(vect=7)
void intTRC( void ){
    trcsr &= 0xfe;

    /* タイマRC　デューティ比の設定 */
    trcgrb = trcgrb_buff;
    trcgrd = trcgrd_buff;
}





/************************************************************************/
/* マイコンボード上のディップスイッチ値読み込み                         */
/* 引数　 なし                                                          */
/* 戻り値 スイッチ値 0～15                                              */
/************************************************************************/
unsigned char dipsw_get( void ){
    unsigned char sw;
    sw = p1 & 0x0f;                     /* P1_3～P1_0読み込み           */
    return sw;
}





/************************************************************************/
/* モータドライブ基板TypeS Ver.3上のプッシュスイッチ値読み込み          */
/* 引数　 なし                                                          */
/* 戻り値 スイッチ値 0:OFF 1:ON                                         */
/************************************************************************/
unsigned char pushsw_get( void ){
    unsigned char sw;
    sw = ~p9_5 & 0x01;
    return sw;
}





/************************************************************************/
/* モータドライブ基板TypeS Ver.3のCN6の状態読み込み                     */
/* 引数　 なし                                                          */
/* 戻り値 0～15                                                         */
/************************************************************************/
unsigned char cn6_get( void ){
    unsigned char data;
    data = p7 >> 4;
    return data;
}





/************************************************************************/
/* モータドライブ基板TypeS Ver.3のLED制御                               */
/* 引数　 8個のLED制御 0:OFF 1:ON                                       */
/* 戻り値 なし                                                          */
/************************************************************************/
void led_out( unsigned char led )
{
    /* 実際の出力はタイマRB割り込み処理で実施 */
    types_led = led;
}





/************************************************************************/
/* 後輪の速度制御                                                       */
/* 引数　 左モータ:-100～100 , 右モータ:-100～100                       */
/*        0で停止、100で正転100%、-100で逆転100%                        */
/* 戻り値 なし                                                          */
/************************************************************************/
void motor_r( int accele_l, int accele_r )
{
    int sw_data;

    sw_data  = 20;         /* ディップスイッチ読み込み     */
    accele_l = accele_l * sw_data / 20;
    accele_r = accele_r * sw_data / 20;

    /* 左後モータ */
    if( accele_l >= 0 ) {
        p2_1 = 0;
        trdgrd0 = (long)( TRD_MOTOR_CYCLE - 2 ) * accele_l / 100;
    } else {
        p2_1 = 1;
        trdgrd0 = (long)( TRD_MOTOR_CYCLE - 2 ) * ( -accele_l ) / 100;
    }

    /* 右後モータ */
    if( accele_r >= 0 ) {
        p2_3 = 0;
        trdgrc1 = (long)( TRD_MOTOR_CYCLE - 2 ) * accele_r / 100;
    } else {
        p2_3 = 1;
        trdgrc1 = (long)( TRD_MOTOR_CYCLE - 2 ) * ( -accele_r ) / 100;
    }
}





/************************************************************************/
/* 前輪の速度制御                                                       */
/* 引数　 左モータ:-100～100 , 右モータ:-100～100                       */
/*        0で停止、100で正転100%、-100で逆転100%                        */
/* 戻り値 なし                                                          */
/************************************************************************/
void motor_f( int accele_l, int accele_r )
{
    int sw_data;

    sw_data  = dipsw_get() + 5;         /* ディップスイッチ読み込み     */
    accele_l = accele_l * sw_data / 20;
    accele_r = accele_r * sw_data / 20;

    /* 左前モータ */
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

    /* 右前モータ */
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
/* 後モータ停止動作（ブレーキ、フリー）                                 */
/* 引数　 左モータ:FREE or BRAKE , 右モータ:FREE or BRAKE               */
/* 戻り値 なし                                                          */
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
/* 前モータ停止動作（ブレーキ、フリー）                                 */
/* 引数　 左モータ:FREE or BRAKE , 右モータ:FREE or BRAKE               */
/* 戻り値 なし                                                          */
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
/* タイマRB 割り込み処理                                                */
/************************************************************************/
#pragma interrupt /B intTRB(vect=24)
void intTRB( void )
{
    unsigned int i;

    asm(" fset I ");                    /* タイマRB以上の割り込み許可   */

    cnt1++;
	cnt2++;
	cnt_cleaning_time++;
	cnt_cleaning_total_time++;
	cnt_touch_time++;
	
    /* ブザー処理 */
    beepProcessS();

    /* 10回中1回実行する処理 */
    iTimer10++;
    switch( iTimer10 ) {
    case 1:
        /* エンコーダ制御 */
        i = trg;
        iEncoder       = i - uEncoderBuff;
        lEncoderTotal += iEncoder;
        uEncoderBuff = i;
        break;

    case 2:
        /* スイッチ読み込み準備 */
        p9_4 = 0;                       /* LED出力OFF                   */
        pd8  = 0x00;
        break;

    case 3:
        /* スイッチ読み込み、LED出力 */
        types_dipsw = ~p8;              /* ﾄﾞﾗｲﾌﾞ基板TypeS Ver.3のSW読み込み*/
        p8  = types_led;                /* ﾄﾞﾗｲﾌﾞ基板TypeS Ver.3のLEDへ出力*/
        pd8 = 0xff;
        p9_4 = 1;                       /* LED出力ON                    */
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
        /* iTimer10変数の処理 */
        iTimer10 = 0;
        break;
    }
}





/************************************************************************/
/* 指定時間移動させる	                                                */
/************************************************************************/
void move_order(int direction,int get_move_time){
	
	switch( direction ){
		
		//左回転
		case MOVE_LEFT:
		motor_f(-100,100);
		break;
		
		//右回転
		case MOVE_RIGHT:
		motor_f(100,-100);
		break;
		
		//直進
		case MOVE_STRAIGHT:
		motor_f(100,100);
		break;
		
		//後進
		case MOVE_BACK:
		motor_f(-100,-100);
		break;
		
		default:
		return;
		
	}
	cnt2 = 0;
	
	//指定時間分動作を継続
	while( cnt2 < get_move_time ){
		if( pushsw_get() == 1 )break;	
	}
	motor_f(0,0);
}





/************************************************************************/
/* end of file                                                          */
/************************************************************************/

/*
改訂経歴

2011.06.01 Ver.1.00 作成
2012.02.23 Ver.1.01 モータドライブ基板TypeS Ver.3と
                    アナログセンサ基板TypeS Ver.2のコメント変更
2013.05.10 Ver.2.00 モータドライブ基板TypeS Ver.4に対応
*/