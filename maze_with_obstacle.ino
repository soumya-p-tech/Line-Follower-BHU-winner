// Technex 2026 - Maze Logic (LSRB) + Obstacle U-Turn

#include <Arduino.h>
#include <QTRSensors.h>

// ======================================================
// MOTOR PINS
// ======================================================
#define ENA 13
#define IN1 18
#define IN2 19
#define IN3 21
#define IN4 22
#define ENB 23

// ======================================================
// OBSTACLE SENSOR
// ======================================================
#define OBSTACLE_PIN 4

// ======================================================
// QTR SENSOR ARRAY
// ======================================================
#define NUM_SENSORS 8
const uint8_t qtrPins[NUM_SENSORS] =
{
    32, 33, 34, 35,
    25, 26, 27, 14
};

QTRSensors qtr;
uint16_t sensorValues[NUM_SENSORS];

// ======================================================
// PWM CONFIG
// ======================================================
#define PWM_LEFT_CH   0
#define PWM_RIGHT_CH  1
#define PWM_FREQ      1000
#define PWM_RES       8

// ======================================================
// PARAMETERS
// ======================================================
#define THRESHOLD 3200

float Kp = 0.16;
float Ki = 0.00;
float Kd = 0.70;

int baseSpeed = 220;
int fastSpeed = 255;

int lastError = 0;
float integral = 0;

// ======================================================
// MAZE VARIABLES
// ======================================================
char path[100];
uint8_t pathLength = 0;

bool mazeSolved = false;
bool replayMode = false;

uint8_t replayIndex = 0;

// ======================================================
// MOTOR CONTROL
// ======================================================
void leftMotor(int speed)
{
    speed = constrain(speed, -255, 255);

    if (speed >= 0)
    {
        digitalWrite(IN1, HIGH);
        digitalWrite(IN2, LOW);
    }
    else
    {
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, HIGH);
        speed = -speed;
    }

    ledcWrite(PWM_LEFT_CH, speed);
}

void rightMotor(int speed)
{
    speed = constrain(speed, -255, 255);

    if (speed >= 0)
    {
        digitalWrite(IN3, HIGH);
        digitalWrite(IN4, LOW);
    }
    else
    {
        digitalWrite(IN3, LOW);
        digitalWrite(IN4, HIGH);
        speed = -speed;
    }

    ledcWrite(PWM_RIGHT_CH, speed);
}

void stopMotors()
{
    ledcWrite(PWM_LEFT_CH, 0);
    ledcWrite(PWM_RIGHT_CH, 0);
}

// ======================================================
// PATH SIMPLIFICATION
// ======================================================
void simplifyPath()
{
    if (pathLength < 3 || path[pathLength - 2] != 'B')
        return;

    int totalAngle = 0;

    for (int i = 1; i <= 3; i++)
    {
        switch (path[pathLength - i])
        {
            case 'R': totalAngle += 90; break;
            case 'L': totalAngle += 270; break;
            case 'B': totalAngle += 180; break;
            case 'S': break;
        }
    }

    totalAngle %= 360;

    switch (totalAngle)
    {
        case 0:   path[pathLength - 3] = 'S'; break;
        case 90:  path[pathLength - 3] = 'R'; break;
        case 180: path[pathLength - 3] = 'B'; break;
        case 270: path[pathLength - 3] = 'L'; break;
    }

    pathLength -= 2;
}

void recordTurn(char turn)
{
    path[pathLength++] = turn;
    simplifyPath();

    Serial.print("Path: ");

    for (int i = 0; i < pathLength; i++)
        Serial.print(path[i]);

    Serial.println();
}

// ======================================================
// AUTO CALIBRATION
// ======================================================
void autoCalibrate()
{
    unsigned long startTime = millis();

    while (millis() - startTime < 4000)
    {
        qtr.calibrate();

        if ((millis() / 300) % 2 == 0)
        {
            leftMotor(120);
            rightMotor(-120);
        }
        else
        {
            leftMotor(-120);
            rightMotor(120);
        }

        delay(20);
    }

    stopMotors();
}

// ======================================================
// LINE POSITION
// ======================================================
uint16_t readLine()
{
    uint32_t weightedSum = 0;
    uint32_t total = 0;

    bool lineDetected = false;

    for (uint8_t i = 0; i < NUM_SENSORS; i++)
    {
        uint16_t val = sensorValues[i];

        if (val > THRESHOLD)
            lineDetected = true;
        else
            val = 0;

        val = 4095 - val;

        weightedSum += (uint32_t)val * (i * 1000);
        total += val;
    }

    if (!lineDetected || total == 0)
        return 0xFFFF;

    return weightedSum / total;
}

// ======================================================
// PID FOLLOW
// ======================================================
void lineFollowPID()
{
    uint16_t position = readLine();

    int error;

    if (position == 0xFFFF)
    {
        error = (lastError > 0) ? 3500 : -3500;
    }
    else
    {
        error = (int)position - 3500;
    }

    integral += error;

    int derivative = error - lastError;

    int correction =
        (Kp * error) +
        (Ki * integral) +
        (Kd * derivative);

    lastError = error;

    int leftSpeed = baseSpeed + correction;
    int rightSpeed = baseSpeed - correction;

    leftMotor(leftSpeed);
    rightMotor(rightSpeed);
}

// ======================================================
// TURN FUNCTIONS
// ======================================================
void turnLeft()
{
    recordTurn('L');

    leftMotor(-180);
    rightMotor(180);

    delay(150);

    unsigned long t = millis();

    while (millis() - t < 1000)
    {
        qtr.read(sensorValues);

        if (sensorValues[3] > THRESHOLD ||
            sensorValues[4] > THRESHOLD)
        {
            break;
        }
    }
}

void turnRight()
{
    recordTurn('R');

    leftMotor(180);
    rightMotor(-180);

    delay(150);

    unsigned long t = millis();

    while (millis() - t < 1000)
    {
        qtr.read(sensorValues);

        if (sensorValues[3] > THRESHOLD ||
            sensorValues[4] > THRESHOLD)
        {
            break;
        }
    }
}

void turnBack()
{
    recordTurn('B');

    leftMotor(180);
    rightMotor(-180);

    delay(350);
}

// ======================================================
// OBSTACLE HANDLING
// ======================================================
void obstacleUTurn()
{
    stopMotors();
    delay(300);

    turnBack();
}

// ======================================================
// GOAL DETECTION
// ======================================================
bool isGoalReached()
{
    int activeCount = 0;

    for (int i = 0; i < NUM_SENSORS; i++)
    {
        if (sensorValues[i] > THRESHOLD)
            activeCount++;
    }

    return (activeCount >= 7);
}

// ======================================================
// FAST REPLAY
// ======================================================
void replayTurn(char dir)
{
    switch (dir)
    {
        case 'L':
            leftMotor(-220);
            rightMotor(220);
            break;

        case 'R':
            leftMotor(220);
            rightMotor(-220);
            break;

        case 'B':
            leftMotor(220);
            rightMotor(-220);
            delay(350);
            break;

        case 'S':
            return;
    }

    delay(150);

    unsigned long t = millis();

    while (millis() - t < 1000)
    {
        qtr.read(sensorValues);

        if (sensorValues[3] > THRESHOLD ||
            sensorValues[4] > THRESHOLD)
        {
            break;
        }
    }
}

void replayShortestPath()
{
    qtr.read(sensorValues);

    bool leftPath =
        sensorValues[5] > THRESHOLD &&
        sensorValues[6] > THRESHOLD &&
        sensorValues[7] > THRESHOLD;

    bool rightPath =
        sensorValues[0] > THRESHOLD &&
        sensorValues[1] > THRESHOLD &&
        sensorValues[2] > THRESHOLD;

    bool junction = leftPath || rightPath;

    if (junction)
    {
        char nextMove = path[replayIndex++];
        replayTurn(nextMove);
    }

    baseSpeed = fastSpeed;

    lineFollowPID();
}

// ======================================================
// SETUP
// ======================================================
void setup()
{
    Serial.begin(115200);

    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);

    pinMode(ENA, OUTPUT);
    pinMode(ENB, OUTPUT);

    pinMode(OBSTACLE_PIN, INPUT_PULLUP);

    ledcSetup(PWM_LEFT_CH, PWM_FREQ, PWM_RES);
    ledcAttachPin(ENA, PWM_LEFT_CH);

    ledcSetup(PWM_RIGHT_CH, PWM_FREQ, PWM_RES);
    ledcAttachPin(ENB, PWM_RIGHT_CH);

    qtr.setTypeAnalog();
    qtr.setSensorPins(qtrPins, NUM_SENSORS);

    autoCalibrate();
}

// ======================================================
// MAIN LOOP
// ======================================================
void loop()
{
    // Obstacle detection
    if (digitalRead(OBSTACLE_PIN) == LOW)
    {
        obstacleUTurn();
        return;
    }

    qtr.read(sensorValues);

    // Goal reached
    if (isGoalReached())
    {
        stopMotors();
        delay(2000);

        if (!mazeSolved)
        {
            mazeSolved = true;
            replayMode = true;

            replayIndex = 0;

            Serial.println("Maze solved!");

            leftMotor(220);
            rightMotor(-220);

            delay(600);

            return;
        }
        else
        {
            stopMotors();

            while (1);
        }
    }

    // Fast replay mode
    if (replayMode)
    {
        replayShortestPath();
        return;
    }

    // Junction detection
    bool rightPath =
        sensorValues[0] > THRESHOLD &&
        sensorValues[1] > THRESHOLD &&
        sensorValues[2] > THRESHOLD;

    bool straightPath =
        (sensorValues[2] > THRESHOLD &&
         sensorValues[3] > THRESHOLD &&
         sensorValues[4] > THRESHOLD)

         ||

        (sensorValues[3] > THRESHOLD &&
         sensorValues[4] > THRESHOLD &&
         sensorValues[5] > THRESHOLD);

    bool leftPath =
        sensorValues[5] > THRESHOLD &&
        sensorValues[6] > THRESHOLD &&
        sensorValues[7] > THRESHOLD;

    // LSRB maze logic
    if (leftPath)
    {
        turnLeft();
        return;
    }

    if (straightPath)
    {
        recordTurn('S');
    }
    else if (rightPath)
    {
        turnRight();
        return;
    }
    else
    {
        turnBack();
        return;
    }

    // PID line following
    lineFollowPID();

    delay(5);
}
