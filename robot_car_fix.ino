const int enA = 11; // Left motor power
const int enB = 9;  // Right motor power
const int inAL = 13; // Motor 1 left 
const int inBL = 12; // Motor 1 left 
const int inCR = 10; // Motor 2 right
const int inDR = 8;  // Motor 2 right

// Ultrasonic
const int trig_pin = 5;
const int echo_pin = 6;

// Tracking sensors
const int sensor1 = A1;
const int sensor2 = A2;
const int sensor3 = A3;
const int sensor4 = A4;
const int sensor5 = A5;

// Light signal sensor
const int ldr_sensor = 7; // Digital pin
int flash_count = 0;
int ldr_value = 0; // LDR reading
bool light_on = false; // Track if the light is currently on
unsigned long last_flash = 0;
unsigned long start_time = 0;
const unsigned long timeout = 3000;
int previous_ldr_value = HIGH;

bool stopped = false;
bool tracking_line = true;
int firstturn_direction = -1;
int intersection_count = 0;

void setup() {
    pinMode(enA, OUTPUT); // Left power
    pinMode(enB, OUTPUT); // Right power
    // Left motor
    pinMode(inAL, OUTPUT); 
    pinMode(inBL, OUTPUT);
    // Right motor
    pinMode(inCR, OUTPUT);
    pinMode(inDR, OUTPUT);

    pinMode(ldr_sensor, INPUT);
    pinMode(sensor1, INPUT);
    pinMode(sensor2, INPUT);
    pinMode(sensor3, INPUT);
    pinMode(sensor4, INPUT);
    pinMode(sensor5, INPUT);

    // Ultrasonic
    pinMode(trig_pin, OUTPUT);
    pinMode(echo_pin, INPUT);

    Serial.begin(9600);
} 

void no_move() {
    digitalWrite(inAL, LOW);
    digitalWrite(inBL, LOW);
    digitalWrite(inCR, LOW);
    digitalWrite(inDR, LOW);

    analogWrite(enA, 0);
    analogWrite(enB, 0);
    delay(500);
}

int ultrasonic_sensor() {
    digitalWrite(trig_pin, LOW);
    delayMicroseconds(2);
    
    // Send Pulse
    digitalWrite(trig_pin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig_pin, LOW);
    
    // Collect Pulse
    long duration = pulseIn(echo_pin, HIGH);
    
    // Distance calculation
    long distance = (duration * 0.034) / 2;

    return distance;
}

void move_forward(int speed = 80) {
    long distance = ultrasonic_sensor();

    if (distance <= 10 && distance >= 2) {
        no_move();
    } else {
        digitalWrite(inAL, LOW);
        digitalWrite(inBL, HIGH);
        digitalWrite(inCR, HIGH);
        digitalWrite(inDR, LOW);

        analogWrite(enA, speed);
        analogWrite(enB, speed);
    }
}

void move_backward(int speed = 70) {
    long distance = ultrasonic_sensor();

    if (distance <= 10 && distance >= 2) {
        no_move();
    } else {
        digitalWrite(inAL, HIGH);
        digitalWrite(inBL, LOW);
        digitalWrite(inCR, LOW);
        digitalWrite(inDR, HIGH);

        analogWrite(enA, speed);
        analogWrite(enB, speed);
    }
}

void turn_right(int speed = 70) {
    long distance = ultrasonic_sensor();

    if (distance <= 10 && distance >= 2) {
        no_move();
    } else {
        digitalWrite(inAL, LOW);
        digitalWrite(inBL, HIGH);
        digitalWrite(inCR, LOW);
        digitalWrite(inDR, LOW);

        analogWrite(enA, speed);
        analogWrite(enB, speed);
    }
}

void turn_left(int speed = 70) {
    long distance = ultrasonic_sensor();

    if (distance <= 10 && distance >= 2) {
        no_move();
    } else {
        digitalWrite(inAL, LOW);
        digitalWrite(inBL, LOW);
        digitalWrite(inCR, HIGH);
        digitalWrite(inDR, LOW);

        analogWrite(enA, speed);
        analogWrite(enB, speed);
    }
}

void dead_turn_left(int duration) {
    long distance = ultrasonic_sensor();

    if (distance <= 10 && distance >= 2) {
        no_move();
    } else {
        digitalWrite(inAL, LOW);
        digitalWrite(inBL, LOW);
        digitalWrite(inCR, HIGH);
        digitalWrite(inDR, LOW);

        analogWrite(enA, 110);
        analogWrite(enB, 110);
        delay(duration);
        no_move();
    }
}

void dead_turn_right(int duration) {
    long distance = ultrasonic_sensor();

    if (distance <= 10 && distance >= 2) {
        no_move();
    } else {
        digitalWrite(inAL, LOW);
        digitalWrite(inBL, HIGH);
        digitalWrite(inCR, LOW);
        digitalWrite(inDR, LOW);

        analogWrite(enA, 110);
        analogWrite(enB, 110);
        delay(duration);
        no_move();
        
    }
}

void ldrsensor() {
    ldr_value = digitalRead(ldr_sensor); 

    if (ldr_value == LOW && !light_on) {
        light_on = true;   
        flash_count++;     
        Serial.print("Flash Count: ");
        Serial.println(flash_count);
        last_flash = millis(); 
        delay(200);  
    } else if (ldr_value == HIGH) {
        light_on = false;  
    }
    if(flash_count > 0 && (millis() - last_flash > timeout))
    {
      if(firstturn_direction == -1)
      {
        if(flash_count == 1)  
        {
          turn_left();
          firstturn_direction = 0;
        }
        else if(flash_count == 2)  
        {
          turn_right();
          firstturn_direction = 1;
        }
        tracking_line = true; //resume line tracking
        delay(100);
      }
      flash_count = 0;  //reset flash count for the next working
    }
}

void tracking_sensors() {
    int sensor1_value = analogRead(sensor1);
    int sensor2_value = analogRead(sensor2);
    int sensor3_value = analogRead(sensor3);
    int sensor4_value = analogRead(sensor4);
    int sensor5_value = analogRead(sensor5);

    // 11011 move forward
    if (sensor1_value > 900 && sensor2_value > 900 && sensor3_value < 100 && sensor4_value > 900 && sensor5_value > 900) {
        move_forward();
    }
    // 10011
    else if (sensor1_value > 900 && sensor2_value < 800 && sensor3_value < 100 && sensor4_value > 900 && sensor5_value > 900) {
        turn_right();
    }
    // 11100
    else if (sensor1_value > 900 && sensor2_value > 900 && sensor3_value > 110 && sensor4_value < 800 && sensor5_value < 800) {
        turn_left();
    }
    // 11000 right turn
    else if (sensor1_value > 900 && sensor2_value > 900 && sensor3_value < 100 && sensor4_value < 800 && sensor5_value < 800) {
        turn_right();
    }
    // 10000 right turn
    else if (sensor1_value > 900 && sensor2_value < 800 && sensor3_value < 100 && sensor4_value < 800 && sensor5_value < 800) {
        turn_right();
    }
    // 11001
    else if (sensor1_value > 900 && sensor2_value > 900 && sensor3_value < 100 && sensor4_value < 800 && sensor5_value > 900) {
        turn_left();
    }
    // 00111
    else if (sensor1_value < 800 && sensor2_value < 800 && sensor3_value > 110 && sensor4_value > 900 && sensor5_value > 900) {
        turn_right();
    }
    // 00011 left turn
    else if (sensor1_value < 800 && sensor2_value < 800 && sensor3_value < 100 && sensor4_value > 900 && sensor5_value > 900) {
        turn_left();
    }
    // 00001 left turn
    else if (sensor1_value < 800 && sensor2_value < 800 && sensor3_value < 100 && sensor4_value < 800 && sensor5_value > 900) {
        turn_left();
    }
     //11010
    else if(sensor1_value > 900 && sensor2_value > 900 && sensor3_value < 100 && sensor4_value > 900 && sensor5_value < 800) 
    {
      move_backward();
      if(sensor1_value > 900 && sensor2_value > 900 && sensor3_value < 100 && sensor4_value > 900 && sensor5_value > 900)
      {
        dead_turn_right(1000);
      }
      
    }
    //01011
    else if(sensor1_value < 800 && sensor2_value > 900 && sensor3_value < 100 && sensor4_value > 900 && sensor5_value > 900)
    {
      move_backward();
      if(sensor1_value > 900 && sensor2_value > 900 && sensor3_value < 100 && sensor4_value > 900 && sensor5_value > 900)
      {
        dead_turn_left(1000);
      }
      
    }
  
    // 11111
    else if (sensor1_value > 900 && sensor2_value > 900 && sensor3_value > 110 && sensor4_value > 900 && sensor5_value > 900) {
        int time_limit = 1000;
        if(start_time == 0)
        {
          start_time = millis();
        }
        unsigned long current_time = millis();
        unsigned long elapsed_time = current_time - start_time;

        if (elapsed_time < time_limit) {
          move_forward();
        } 
        else {
          move_backward(1000);
          start_time = 0;  // Reset the timer after moving backward
        }
    start_time = 0;
}
    // 00000 - no line detected, robot should stop and wait for flash count
    else if (sensor1_value < 800 && sensor2_value < 800 && sensor3_value < 100  && sensor4_value < 800 && sensor5_value < 800) {
        intersection_count++;
        no_move(); // Stop the robot

        if (intersection_count == 1) {
            while (flash_count == 0) {
                ldrsensor();
            }
            if (flash_count == 1) {
                turn_left();
                firstturn_direction = 0;
            } else if (flash_count == 2) {
                turn_right();
                firstturn_direction = 1;
            }
            flash_count = 0;
        } else if (intersection_count == 2) {
            if (firstturn_direction == 0) {
                turn_left();
            } else if (firstturn_direction == 1) {
                turn_right();
            }
        } else if (intersection_count == 3) {
            no_move(); // Stop completely at the
while (true) {
                // Keep the robot stopped indefinitely or add other logic
            }
        }
    }
}

void loop() {
    if (stopped) {
        // If the robot is stopped, don't do anything else
        return;
    }

    // Continuously read the tracking sensors and LDR sensor
    tracking_sensors();
}