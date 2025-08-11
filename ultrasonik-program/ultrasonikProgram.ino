#define TRIG1 7
#define ECHO1 6

#define TRIG2 9
#define ECHO2 8

#define TRIG3 11
#define ECHO3 10

#define TRIG4 13
#define ECHO4 12


struct distanceSet {
  float very_close;
  float close;
  float medium;
  float far;
};

struct FuzzyOutputPreference {
  float turn_left;
  float straight;
  float turn_right;
  float backward;
};

float trimf(float x, float a, float b, float c) {
  if (x <= a || x >= c) return 0.0;
  else if (x <= b) return (x - a) / (b - a);
  else return (c - x) / (c - b);
}

distanceSet fuzzifyDistance(long distance) {
  distanceSet ds;
  ds.very_close = trimf(distance, 0, 0, 20);
  ds.close = trimf(distance, 10, 30, 50);
  ds.medium = trimf(distance, 40, 60, 80);
  ds.far = trimf(distance, 70, 100, 100);
  return ds;
}

float fuzzyMin(float a, float b) {
  return (a < b) ? a : b;
}

float fuzzyMax(float a, float b) {
  return (a > b) ? a : b;
}

float fuzzyMax3(float a, float b, float c) {
  return fuzzyMax(a, fuzzyMax(b, c));
}

long readUltrasonic(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 30000);
  if (duration == 0) return 100;
  long distance = duration * 0.034 / 2;
  return constrain(distance, 0, 100);
}

FuzzyOutputPreference fuzzyInference(long d1, long d2, long d3, long d4) {
  distanceSet fdk = fuzzifyDistance(d1);
  distanceSet fdk_ = fuzzifyDistance(d2);
  distanceSet fsk = fuzzifyDistance(d3);
  distanceSet fsk_ = fuzzifyDistance(d4);

  float pref_turn_left = 0.0;
  float pref_straight = 0.0;
  float pref_turn_right = 0.0;
  float pref_backward = 0.0;

  float activation_rule1 = fuzzyMin(fdk.far, fdk_.far);
  pref_straight = fuzzyMax(pref_straight, activation_rule1);

  float activation_rule2 = fuzzyMin(fdk.close, fdk_.far);
  pref_turn_right = fuzzyMax(pref_turn_right, activation_rule2);

  float activation_rule3 = fuzzyMin(fdk_.close, fdk.far);
  pref_turn_left = fuzzyMax(pref_turn_left, activation_rule3);

  float activation_rule4 = fuzzyMax(fdk.very_close, fdk_.very_close);
  pref_straight = fuzzyMax(pref_straight, activation_rule4);

  float activation_rule5 = fuzzyMin(fuzzyMax(fdk.close, fdk_.close), fsk.far);
  pref_turn_left = fuzzyMax(pref_turn_left, activation_rule5);

  float activation_rule6 = fuzzyMin(fuzzyMax(fdk.close, fdk_.close), fsk_.far);
  pref_turn_right = fuzzyMax(pref_turn_right, activation_rule6);

  float activation_rule7 = fuzzyMin(fdk.close, fuzzyMin(fdk_.close, fuzzyMin(fsk.close, fsk_.close)));
  pref_straight = fuzzyMax(pref_straight, activation_rule7);

  float activation_rule8 = fuzzyMax(fdk.very_close, fdk_.very_close);
  activation_rule8 = fuzzyMin(activation_rule8, fuzzyMin(fsk.very_close, fsk_.very_close));
  pref_backward = fuzzyMax(pref_backward, activation_rule8);

  FuzzyOutputPreference fo;
  fo.turn_left = pref_turn_left;
  fo.straight = pref_straight;
  fo.turn_right = pref_turn_right;
  fo.backward = pref_backward;

  return fo;
}

const int OUTPUT_UNIVERSE_SIZE = 21;
const float OUTPUT_UNIVERSE_MIN = -15.0;
const float OUTPUT_UNIVERSE_MAX = 10.0;

float mf_turn_left_out(float x) {
  return trimf(x, -10, -9.999, 0);
}
float mf_straight_out(float x) {
  return trimf(x, -5, 0, 5);
}
float mf_turn_right_out(float x) {
  return trimf(x, 0, 10, 10);
}

float mf_backward_out(float x) {
  return trimf(x, -15, -12, -10); 
}

float defuzzifyCOG(FuzzyOutputPreference output) {
  float numerator = 0.0;
  float denominator = 0.0;

  for (int i = 0; i < OUTPUT_UNIVERSE_SIZE; i++) {
    float x = OUTPUT_UNIVERSE_MIN + i * (OUTPUT_UNIVERSE_MAX - OUTPUT_UNIVERSE_MIN) / (OUTPUT_UNIVERSE_SIZE - 1);

    float combined = fuzzyMax(
      fuzzyMax3(
        fuzzyMin(output.turn_left, mf_turn_left_out(x)),
        fuzzyMin(output.straight, mf_straight_out(x)),
        fuzzyMin(output.turn_right, mf_turn_right_out(x))
      ),
      fuzzyMin(output.backward, mf_backward_out(x)) 
    );

    numerator += x * combined;
    denominator += combined;
  }

  if (denominator == 0) return 0.0;
  return numerator / denominator;
}

long d1, d2, d3, d4;

void setup() {
  Serial.begin(9600);

  pinMode(TRIG1, OUTPUT); pinMode(ECHO1, INPUT);
  pinMode(TRIG2, OUTPUT); pinMode(ECHO2, INPUT);
  pinMode(TRIG3, OUTPUT); pinMode(ECHO3, INPUT);
  pinMode(TRIG4, OUTPUT); pinMode(ECHO4, INPUT);

}

void loop() {
  d1 = readUltrasonic(TRIG1, ECHO1);
  d2 = readUltrasonic(TRIG2, ECHO2);
  d3 = readUltrasonic(TRIG3, ECHO3);
  d4 = readUltrasonic(TRIG4, ECHO4);

  FuzzyOutputPreference result = fuzzyInference(d1, d2, d3, d4);
  float final_value = defuzzifyCOG(result);

  enum DIRECTION {
    BACKWARD = 0,
    LEFT = 1,
    RIGHT = 2,
    STRAIGHT_FORWARD = 3
  };

  DIRECTION direction = STRAIGHT_FORWARD;
  if (final_value < -10.0) {
    direction = BACKWARD ;
  } else if (final_value < -2.0) {
    direction = LEFT;
  } else if (final_value > 2.0) {
    direction = RIGHT;
  } else {
    direction = STRAIGHT_FORWARD;
  }

  Serial.print(d1); Serial.print(",");
  Serial.print(d2); Serial.print(",");
  Serial.print(d3); Serial.print(",");
  Serial.print(d4); Serial.print(",");
  Serial.println((int)direction); 

  delay(200);
}