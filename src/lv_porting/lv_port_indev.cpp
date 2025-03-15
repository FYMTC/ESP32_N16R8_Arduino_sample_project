/**
 * @file lv_port_indev_templ.c
 *
 */

/*Copy this file as "lv_port_indev.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_indev.h"
// #include "../../lvgl.h"
#include "lvgl.h"
#define MY_DISP_HOR_RES 240 // 屏幕宽度
#define MY_DISP_VER_RES 320 // 屏幕高度
/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
extern void timer_page0_rest_cb(lv_event_t *);
static void touchpad_init(void);
static void touchpad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data);
static bool touchpad_is_pressed(void);
static void touchpad_get_xy(lv_coord_t *x, lv_coord_t *y);

static void mouse_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data);
static bool mouse_is_pressed(void);
static void mouse_get_xy(lv_coord_t *x, lv_coord_t *y);
int8_t mouseX = 0;
int8_t mouseY = 0;
bool mouseLeftButton = false;
uint8_t notifyCallback_statue = 0;
static int last_notifyCallback_statue = 0;
bool mouse_indev_statue = true;

static void keypad_init(void);
static void keypad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data);
static uint32_t keypad_get_key(void);

static void encoder_init(void);
static void encoder_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data);
static void encoder_handler(void);

// 编码器计数变量
volatile int16_t encoder_count = 0;
volatile bool button_pressed = false;

static void button_init(void);
static void button_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data);
static int8_t button_get_pressed_id(void);
static bool button_is_pressed(uint8_t id);

/**********************
 *  STATIC VARIABLES
 **********************/
lv_indev_t *indev_touchpad;
lv_indev_t *indev_mouse;
lv_indev_t *indev_keypad;
lv_indev_t *indev_encoder;
lv_indev_t *indev_button;

static int32_t encoder_diff;
static lv_indev_state_t encoder_state;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
// 读取编码器计数
int16_t read_encoder()
{
    int16_t count;
    pcnt_get_counter_value(PCNT_UNIT, &count);
    pcnt_counter_clear(PCNT_UNIT);
    return count;
}

// 按钮中断处理
void IRAM_ATTR button_isr()
{
    button_pressed = true;
}

void lv_port_indev_init(void)
{
    /**
     * Here you will find example implementation of input devices supported by LittelvGL:
     *  - Touchpad
     *  - Mouse (with cursor support)
     *  - Keypad (supports GUI usage only with key)
     *  - Encoder (supports GUI usage only with: left, right, push)
     *  - Button (external buttons to press points on the screen)
     *
     *  The `..._read()` function are only examples.
     *  You should shape them according to your hardware
     */

    /*------------------
     * Touchpad
     * -----------------*/

    /*Initialize your touchpad if you have*/

    /*Register a touchpad input device*/

    /*------------------
     * Mouse
     * -----------------*/
    static lv_indev_drv_t mouse_indev_drv;
    // /*Register a mouse input device*/
    lv_indev_drv_init(&mouse_indev_drv);
    mouse_indev_drv.type = LV_INDEV_TYPE_POINTER;
    mouse_indev_drv.read_cb = mouse_read;
    indev_mouse = lv_indev_drv_register(&mouse_indev_drv);

    lv_obj_t *img_mouse_pointer = lv_label_create(lv_scr_act());
    LV_FONT_DECLARE(NotoSansSC_Medium_3500);
    lv_obj_set_style_text_font(img_mouse_pointer, &NotoSansSC_Medium_3500, 0);
    lv_label_set_text(img_mouse_pointer, "\xEF\x89\x85"); //"\xEF\x89\x85"使用自定义字体里的鼠标符号
    lv_indev_set_cursor(indev_mouse, img_mouse_pointer);

    // /*------------------
    //  * Keypad
    //  * -----------------*/

    // /*Initialize your keypad or keyboard if you have*/
    // keypad_init();

    // /*Register a keypad input device*/
    // lv_indev_drv_init(&indev_drv);
    // indev_drv.type = LV_INDEV_TYPE_KEYPAD;
    // indev_drv.read_cb = keypad_read;
    // indev_keypad = lv_indev_drv_register(&indev_drv);

    // /*Later you should create group(s) with `lv_group_t * group = lv_group_create()`,
    //  *add objects to the group with `lv_group_add_obj(group, obj)`
    //  *and assign this input device to group to navigate in it:
    //  *`lv_indev_set_group(indev_keypad, group);`*/

    // /*------------------
    //  * Encoder
    //  * -----------------*/

    // /*Initialize your encoder if you have*/
    encoder_init();

    // /*Register a encoder input device*/
    static lv_indev_drv_t indev_drv_encoder;
    lv_indev_drv_init(&indev_drv_encoder);
    indev_drv_encoder.type = LV_INDEV_TYPE_ENCODER;
    indev_drv_encoder.read_cb = encoder_read;
    lv_indev_t *encoder_indev = lv_indev_drv_register(&indev_drv_encoder);

    // /*Later you should create group(s) with `lv_group_t * group = lv_group_create()`,
    //  *add objects to the group with `lv_group_add_obj(group, obj)`
    //  *and assign this input device to group to navigate in it:
    //  *`lv_indev_set_group(indev_encoder, group);`*/
    // /*------------------
    //  * Button
    //  * -----------------*/

    // /*Initialize your button if you have*/
    button_init();

    // /*Register a button input device*/
    // 初始化按钮输入设备
    static lv_indev_drv_t indev_drv_button;
    lv_indev_drv_init(&indev_drv_button);
    indev_drv_button.type = LV_INDEV_TYPE_BUTTON;
    indev_drv_button.read_cb = button_read;
    lv_indev_t *button_indev = lv_indev_drv_register(&indev_drv_button);

    // /*Assign buttons to points on the screen*/
    // static const lv_point_t btn_points[2] = {
    //     {10, 10},   /*Button 0 -> x:10; y:10*/
    //     {40, 100},  /*Button 1 -> x:40; y:100*/
    // };
    // lv_indev_set_button_points(indev_button, btn_points);

    // 将按钮与编码器输入设备关联
    static lv_point_t points_array[] = {{0, 0}};
    lv_indev_set_button_points(button_indev, points_array);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*------------------
 * Touchpad
 * -----------------*/

/*Initialize your touchpad*/
static void touchpad_init(void)
{
    /*Your code comes here*/
}

/*Will be called by the library to read the touchpad*/
static void touchpad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    // static lv_coord_t last_x = 0;
    // static lv_coord_t last_y = 0;

    // static int x_pos = 0;
    // static int y_pos = 0;

    /*Save the pressed coordinates and the state*/
    // if(touchpad_is_pressed()) {
    //     touchpad_get_xy(&last_x, &last_y);
    //     data->state = LV_INDEV_STATE_PR;
    // }
    // else {
    //     data->state = LV_INDEV_STATE_REL;
    // }

    /*Set the last pressed coordinates*/
    // data->point.x = last_x;
    // data->point.y = last_y;
}

/*Return true is the touchpad is pressed*/

/*Get the x and y coordinates if the touchpad is pressed*/

/*------------------
 * Mouse
 * -----------------*/

/*Initialize your mouse*/

/*Will be called by the library to read the mouse*/
static void mouse_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    /*Get the current x and y coordinates*/
    static lv_point_t last_pos = {0, 0}; // 保存鼠标的最后位置
    if (last_notifyCallback_statue != notifyCallback_statue)
    {
        last_pos.x += mouseX;
        last_pos.y += mouseY;
    }
    if (!mouse_indev_statue)
    {
        last_pos.x = 0;
        last_pos.y = 0;
    }
    if (last_pos.y > MY_DISP_VER_RES)
    {
        last_pos.y = MY_DISP_VER_RES;
    }
    if (last_pos.x > MY_DISP_HOR_RES)
    {
        last_pos.x = MY_DISP_HOR_RES;
    }
    if (last_pos.x < 0)
    {
        last_pos.x = 0;
    }
    if (last_pos.y < 0)
    {
        last_pos.y = 0;
    }

    last_notifyCallback_statue = notifyCallback_statue;

    data->point = last_pos;
    data->state = mouseLeftButton ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
}

/*------------------
 * Keypad
 * -----------------*/

/*Initialize your keypad*/
static void keypad_init(void)
{
    /*Your code comes here*/
}

/*Will be called by the library to read the mouse*/
static void keypad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    static uint32_t last_key = 0;

    /*Get the current x and y coordinates*/
    mouse_get_xy(&data->point.x, &data->point.y);

    /*Get whether the a key is pressed and save the pressed key*/
    uint32_t act_key = keypad_get_key();
    if (act_key != 0)
    {
        data->state = LV_INDEV_STATE_PR;

        /*Translate the keys to LVGL control characters according to your key definitions*/
        switch (act_key)
        {
        case 1:
            act_key = LV_KEY_NEXT;
            break;
        case 2:
            act_key = LV_KEY_PREV;
            break;
        case 3:
            act_key = LV_KEY_LEFT;
            break;
        case 4:
            act_key = LV_KEY_RIGHT;
            break;
        case 5:
            act_key = LV_KEY_ENTER;
            break;
        }

        last_key = act_key;
    }
    else
    {
        data->state = LV_INDEV_STATE_REL;
    }

    data->key = last_key;
}

/*Get the currently being pressed key.  0 if no key is pressed*/
static uint32_t keypad_get_key(void)
{
    /*Your code comes here*/

    return 0;
}

/*------------------
 * Encoder
 * -----------------*/

/*Initialize your keypad*/
static void encoder_init(void)
{
    /*Your code comes here*/
    pcnt_config_t pcnt_config = {
        .pulse_gpio_num = ENCODER_A_PIN,
        .ctrl_gpio_num = ENCODER_B_PIN,
        .lctrl_mode = PCNT_MODE_REVERSE,
        .hctrl_mode = PCNT_MODE_KEEP,
        .pos_mode = PCNT_COUNT_INC,
        .neg_mode = PCNT_COUNT_DEC,
        .counter_h_lim = INT16_MAX,
        .counter_l_lim = INT16_MIN,
        .unit = PCNT_UNIT,
        .channel = PCNT_CHANNEL,
    };
    pcnt_unit_config(&pcnt_config);

    pcnt_counter_pause(PCNT_UNIT);
    pcnt_counter_clear(PCNT_UNIT);
    pcnt_counter_resume(PCNT_UNIT);
}

/*Will be called by the library to read the encoder*/
static void encoder_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    int16_t count = read_encoder();
    data->enc_diff = count;           // 设置编码器的差值
    data->state = LV_INDEV_STATE_REL; // 默认状态为释放
}

/*Call this function in an interrupt to process encoder events (turn, press)*/
static void encoder_handler(void)
{
    /*Your code comes here*/

    encoder_diff += 0;
    encoder_state = LV_INDEV_STATE_REL;
}

/*------------------
 * Button
 * -----------------*/

/*Initialize your buttons*/
static void button_init(void)
{
    /*Your code comes here*/
    // 初始化按钮
    pinMode(ENCODER_BT_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(ENCODER_BT_PIN), button_isr, FALLING);
}

/*Will be called by the library to read the button*/
static void button_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    data->state = button_pressed ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
    button_pressed = false;
}

/*Get ID  (0, 1, 2 ..) of the pressed button*/
static int8_t button_get_pressed_id(void)
{
    uint8_t i;

    /*Check to buttons see which is being pressed (assume there are 2 buttons)*/
    for (i = 0; i < 2; i++)
    {
        /*Return the pressed button's ID*/
        if (button_is_pressed(i))
        {
            return i;
        }
    }

    /*No button pressed*/
    return -1;
}

/*Test if `id` button is pressed or not*/
static bool button_is_pressed(uint8_t id)
{

    /*Your code comes here*/

    return false;
}

#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
