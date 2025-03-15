// v1.0.0 を有効にします(v0からの移行期間の特別措置です。これを書かない場合は旧v0系で動作します。)
#define LGFX_USE_V1
#define LGFX_tft_USE_P8 0
#include <LovyanGFX.hpp>
#include <driver/i2s.h>
/// 独自の設定を行うクラスを、LGFX_Deviceから派生して作成します。
class LGFX_tft : public lgfx::LGFX_Device
{
  /*
   クラス名は"LGFX"から別の名前に変更しても構いません。
   AUTODETECTと併用する場合は"LGFX"は使用されているため、LGFX以外の名前に変更してください。
   また、複数枚のパネルを同時使用する場合もそれぞれに異なる名前を付けてください。
   ※ クラス名を変更する場合はコンストラクタの名前も併せて同じ名前に変更が必要です。

   名前の付け方は自由に決めて構いませんが、設定が増えた場合を想定し、
   例えばESP32 DevKit-CでSPI接続のILI9341の設定を行った場合、
    LGFX_DevKitC_SPI_ILI9341
   のような名前にし、ファイル名とクラス名を一致させておくことで、利用時に迷いにくくなります。
  //*/

  // 接続するパネルの型にあったインスタンスを用意します。
  // lgfx::Panel_GC9A01      _panel_instance;
  // lgfx::Panel_GDEW0154M09 _panel_instance;
  // lgfx::Panel_HX8357B     _panel_instance;
  // lgfx::Panel_HX8357D     _panel_instance;
  // lgfx::Panel_ILI9163     _panel_instance;
  // lgfx::Panel_ILI9341     _panel_instance;
  // lgfx::Panel_ILI9342     _panel_instance;
  // lgfx::Panel_ILI9481     _panel_instance;
  // lgfx::Panel_ILI9486     _panel_instance;
  // lgfx::Panel_ILI9488     _panel_instance;
  // lgfx::Panel_IT8951      _panel_instance;
  // lgfx::Panel_RA8875      _panel_instance;
  // lgfx::Panel_SH110x      _panel_instance; // SH1106, SH1107
  // lgfx::Panel_SSD1306     _panel_instance;
  // lgfx::Panel_SSD1327     _panel_instance;
  // lgfx::Panel_SSD1331     _panel_instance;
  // lgfx::Panel_SSD1351     _panel_instance; // SSD1351, SSD1357
  // lgfx::Panel_SSD1963     _panel_instance;
  // lgfx::Panel_ST7735      _panel_instance;
  // lgfx::Panel_ST7735S     _panel_instance;
  lgfx::Panel_ST7789 _panel_instance;
  // lgfx::Panel_ST7796      _panel_instance;

#if LGFX_tft_USE_P8
  lgfx::Bus_Parallel8 _bus_instance; // 8位并行总线实例(仅ESP32)
#else
  lgfx::Bus_SPI _bus_instance; // SPIバスのインスタンス
#endif

// lgfx::Bus_I2C       _bus_instance;   // I2Cバスのインスタンス (ESP32のみ)
  // バックライト制御が可能な場合はインスタンスを用意します。(必要なければ削除)
  lgfx::Light_PWM _light_instance;

public:
  // コンストラクタを作成し、ここで各種設定を行います。
  // クラス名を変更した場合はコンストラクタも同じ名前を指定してください。
  LGFX_tft(void)
  {
    { // 进行总线控制的设置。
#if LGFX_tft_USE_P8
      // 配置并行总线控制
      auto cfg = _bus_instance.config();

      cfg.freq_write = 20000000; // 写入频率
     
      cfg.pin_wr = 21;            // WR 引脚
      cfg.pin_rd = 20;            // RD 引脚
      cfg.pin_rs = 1;           // RS(D/C) 引脚
      cfg.pin_d0 = 7;           // 数据引脚 D0
      cfg.pin_d1 = 18;           // 数据引脚 D1
      cfg.pin_d2 = 8;           // 数据引脚 D2
      cfg.pin_d3 = 3;           // 数据引脚 D3
      cfg.pin_d4 = 12;           // 数据引脚 D4
      cfg.pin_d5 = 9;           // 数据引脚 D5
      cfg.pin_d6 = 10;           // 数据引脚 D6
      cfg.pin_d7 = 13;           // 数据引脚 D7
#else
      auto cfg = _bus_instance.config(); // バス設定用の構造体を取得します。

      // SPIバスの設定
      cfg.spi_host = SPI3_HOST; // 使用するSPIを選択  ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
      // ※ ESP-IDFバージョンアップに伴い、VSPI_HOST , HSPI_HOSTの記述は非推奨になるため、エラーが出る場合は代わりにSPI2_HOST , SPI3_HOSTを使用してください。
      cfg.spi_mode = 3;                  // SPI通信モードを設定 (0 ~ 3)
      cfg.freq_write = 80000000;         // 传输时的SPI时钟(最大80mhz，由80mhz除以整数得到
      cfg.freq_read = 16000000;          // 受信時のSPIクロック
      cfg.spi_3wire = false;             // 用MOSI引脚进行接收时设置为真
      cfg.use_lock = false;              // 如果使用事务锁，设置为真
      cfg.dma_channel = 1; // 使用するDMAチャンネルを設定 (0=DMA不使用 / 1=1ch / 2=ch / SPI_DMA_CH_AUTO=自動設定)
      // ※ ESP-IDFバージョンアップに伴い、DMAチャンネルはSPI_DMA_CH_AUTO(自動設定)が推奨になりました。1ch,2chの指定は非推奨になります。
      cfg.pin_sclk = 13; // SPIのSCLKピン番号を設定
      cfg.pin_mosi = 15; // SPIのMOSIピン番号を設定
      cfg.pin_miso = -1; // SPIのMISOピン番号を設定 (-1 = disable)
      cfg.pin_dc = 14;   // SPIのD/Cピン番号を設定  (-1 = disable)
                         // SDカードと共通のSPIバスを使う場合、MISOは省略せず必ず設定してください。
#endif

      _bus_instance.config(cfg);              // 設定値をバスに反映します。
      _panel_instance.setBus(&_bus_instance); // バスをパネルにセットします。
    }

    {                                      // 表示パネル制御の設定を行います。
      auto cfg = _panel_instance.config(); // 表示パネル設定用の構造体を取得します。

      cfg.pin_cs = 12;    // CSが接続されているピン番号   (-1 = disable)
      cfg.pin_rst = 11;  // RSTが接続されているピン番号  (-1 = disable)
      cfg.pin_busy = -1; // BUSYが接続されているピン番号 (-1 = disable)

      // ※ 以下の設定値はパネル毎に一般的な初期値が設定されていますので、不明な項目はコメントアウトして試してみてください。

      cfg.panel_width = 240;    // 实际可显示的宽度
      cfg.panel_height = 320;   // 实际可显示的高度
      cfg.offset_x = 0;         // 面板的X方向偏移量
      cfg.offset_y = 0;         // 面板的Y方向偏移量
      cfg.offset_rotation = 0;  // 旋转方向的值偏移0-7(4-7为上下反转
      cfg.dummy_read_pixel = 8; // 读取像素数据之前需要进行的无效读取操作的像素数量,消除可能存在的前置数据或杂讯
      cfg.dummy_read_bits = 1;  // 在读取数据时需要额外读取的无效位数量,对齐数据流，或者消除噪声或错误数据
      cfg.readable = false;     // 如果可以读取数据，设置为真1/
      cfg.invert = true;        // 如果面板的明暗反转设置为真
      cfg.rgb_order = false;    // 如果面板的红蓝互换，设置为真
      cfg.dlen_16bit = false;   // 如果面板使用16bit并行或SPI以16bit为单位发送数据长度，则设置为真
      cfg.bus_shared = false;   // 如果你和SD卡共享总线，你可以设置为true(通过drawJpgFile等来控制总线)

      // 以下はST7735やILI9163のようにピクセル数が可変のドライバで表示がずれる場合にのみ設定してください。
      //    cfg.memory_width     =   128;  // ドライバICがサポートしている最大の幅
      //    cfg.memory_height    =   64;  // ドライバICがサポートしている最大の高さ

      _panel_instance.config(cfg);
    }

    //*
    {                                      // バックライト制御の設定を行います。（必要なければ削除）
      auto cfg = _light_instance.config(); // バックライト設定用の構造体を取得します。

      cfg.pin_bl = 21;     // バックライトが接続されているピン番号
      cfg.invert = false;  // バックライトの輝度を反転させる場合 true
      cfg.freq = 44100;    // バックライトのPWM周波数
      cfg.pwm_channel = 7; // 使用するPWMのチャンネル番号

      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance); // バックライトをパネルにセットします。
    }
    //*/

    setPanel(&_panel_instance); // 使用するパネルをセットします。
  }
};