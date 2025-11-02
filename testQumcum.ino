#include <Wire.h>

#define SERVO_I2C_ADDR 8  // サーボ制御ボードのI2Cアドレス

#define HEAD 4;

// -------------------------
// I2Cでサーボボードにコマンドを送信する関数
// -------------------------
void sendCommandToServoBoard(const uint8_t* data, int size) {
    Wire.beginTransmission(SERVO_I2C_ADDR);
    for (int i = 0; i < size; i++) {
        Wire.write(data[i]);
    }
    Wire.endTransmission();
}

// -------------------------
// I2Cでサーボボードからデータを受信する関数
// -------------------------
uint16_t receiveDataFromServoBoard(uint8_t* buffer, int size) {
    uint16_t bytesRead = 0;
    Wire.requestFrom(SERVO_I2C_ADDR, size);
    while (Wire.available()) {
        buffer[bytesRead++] = Wire.read();
    }
    return bytesRead;
}

// -------------------------
// サーボボードの初期化（動作モード設定とPWM有効化）
// -------------------------
void initializeServoBoard() {
    uint8_t cmd[10] = {0};

    // 動作モード設定
    cmd[0] = 0; // コマンド番号
    cmd[1] = 1; // スイッチON
    sendCommandToServoBoard(cmd, 2);

    // PWM出力を有効化
    memset(cmd, 0, sizeof(cmd));
    cmd[0] = 2;  // コマンド番号
    cmd[1] = HEAD;  // 出力ON
    *(uint16_t*)(&(cmd[2])) = 1; // 有効化
    sendCommandToServoBoard(cmd, 4);

    delay(100); // 少し待つ

    // PWM出力を有効化
    memset(cmd, 0, sizeof(cmd));
    cmd[0] = 2;  // コマンド番号
    cmd[1] = 4;  // 出力ON
    *(uint16_t*)(&(cmd[2])) = 1; // 有効化
    sendCommandToServoBoard(cmd, 4);

    delay(100); // 少し待つ
}

// -------------------------
// サーボの角度を設定する関数
// ch: サーボ番号, angle: 角度（1/10度単位）, duration: 動作時間(ms)
// -------------------------
void setServoAngle(uint8_t ch, uint16_t angle, uint16_t duration) {
    uint8_t cmd[10] = {0};

    // 角度設定コマンド
    cmd[0] = 4;       // コマンド番号
    cmd[1] = ch;      // サーボチャンネル
    *(uint16_t*)(&(cmd[2])) = angle;    // 角度
    *(uint16_t*)(&(cmd[4])) = duration; // 動作時間
    sendCommandToServoBoard(cmd, 6);

    // サーボ一斉スタート
    memset(cmd, 0, sizeof(cmd));
    cmd[0] = 1;  // スタートコマンド
    sendCommandToServoBoard(cmd, 1);
}

// -------------------------
// サーボの現在角度を取得する関数
// ch: サーボ番号, 戻り値: 角度（1/10度単位）
// -------------------------
uint16_t getServoAngle(uint8_t ch) {
    uint8_t cmd[10] = {0};
    uint8_t reply[10] = {0};

    cmd[0] = 6; // 現在角度取得コマンド
    cmd[1] = ch;
    sendCommandToServoBoard(cmd, 2);

    receiveDataFromServoBoard(reply, 2);
    return *(uint16_t*)reply;
}

// -------------------------
// サーボを動かして、動作完了後に角度を表示する関数
// -------------------------
void moveServoAndCheck(uint8_t ch, uint16_t angle, uint16_t duration) {
    setServoAngle(ch, angle, duration);      // 角度を設定
    delay(duration + 100);                    // サーボが動き終わるまで待機
    uint16_t currentAngle = getServoAngle(ch); // 現在の角度を取得
    Serial.print("現在の角度: ");
    Serial.println(currentAngle / 10.0);      // 1/10度単位なので10で割って表示
}

void setup() {
    Serial.begin(115200);

    Wire.setClock(100000); // I2Cの速度を100kHzに設定
    Wire.begin();           // I2C開始

    initializeServoBoard(); // サーボボード初期化
}

void loop() {
    // サーボを0度に動かす
    moveServoAndCheck(4, 0, 1000);

    // サーボを180度に動かす
    moveServoAndCheck(4, 1800, 1000);

    // 移動中の角度を10回チェック
    for (int i = 0; i < 10; i++) {
        delay(100);
        uint16_t currentAngle = getServoAngle(4);
        Serial.print("移動中の角度: ");
        Serial.println(currentAngle / 10.0);
    }

    delay(400);

    // サーボを90度に動かす
    moveServoAndCheck(4, 900, 1000);

    delay(1000); // 次のループまで待機
}
