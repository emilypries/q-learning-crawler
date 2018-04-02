#include <Servo.h>

Servo inner; // inner joint
Servo outer; // outer joint

struct state {
  double up;
  double down;
  double left;
  double right;
};

struct ds {
  int o;
  int i;
  double q;
};

state Q[36];
double score = 0;
int current_outer = 0;
int current_inner = 0;

ds current_max;
double old_val;
double pot_val;
double reward;

double discount = .5;
double alpha = .1;

const double WALK_REWARD = -.06;
const int MAX_JOINT_VAL = 6;
const int MIN_JOINT_VAL = 0;

void move(int din, int dout){
  int new_out = current_outer + dout;
  int new_in = current_inner + din;

  if ((new_out >= MIN_JOINT_VAL) && (new_out <= MAX_JOINT_VAL) && (new_in >= MIN_JOINT_VAL) && (new_in <= MAX_JOINT_VAL)){
    current_outer = new_out;
    outer.write(current_outer*25);
    current_inner = new_in;
    inner.write(current_inner*25);
  }
}

ds getMax(int i, int o){
  int z = (i*6) + o;
  ds new_move;
  new_move.q = Q[z].up;
  new_move.i = -1;
  new_move.o = 0; 
  Serial.print("UP");
  if (Q[z].down > new_move.q){
    new_move.q = Q[z].down;
    new_move.i = 1;
    new_move.o = 0;
    Serial.print("DOWN");
  }
  if (Q[z].left > new_move.q){
    new_move.q = Q[z].left;
    new_move.i = 0;
    new_move.o = -1;
    Serial.print("LEFT");
  }
  if (Q[z].right > new_move.q){
    new_move.q = Q[z].right;
    new_move.i = 0;
    new_move.o = 1;
    Serial.print("RIGHT");
  }
  return new_move;
}

void update_Q(){
  if (current_max.i == 0){
    if (current_max.o == 1) {
      Q[(current_inner*6)+current_outer].right *= (1 - alpha);
      Q[(current_inner*6)+current_outer].right += alpha + (reward + (discount * current_max.q));
    }
    else {
      Q[(current_inner*6)+current_outer].left *= (1 - alpha);
      Q[(current_inner*6)+current_outer].left += (reward + (discount * current_max.q));
    }
  } else {
    if (current_max.i == -1) {
      Q[(current_inner*6)+current_outer].up *= (1 - alpha);
      Q[(current_inner*6)+current_outer].up += alpha + (reward + (discount * current_max.q));
    }
    if (current_max.i == 1) {
      //Q[(current_inner*6)+current_outer].down *= (1 - alpha);
      Q[(current_inner*6)+current_outer].down += (reward + (discount * current_max.q));
    }
  }
  
}

void setup() {
  // put your setup code here, to run once:
  inner.attach(8);
  outer.attach(9);
  pinMode(A0, INPUT); //Optional 

  for (int i = 0; i < 6; i++){
    for (int j = 0; j < 6; j++) {
      Q[(i*6)+j] = {.1, .1, .1, .1};
      if (i==0) {
        Q[(i*6)+j].up = -10000;
      }
      if (i==5) {
        Q[(i*6)+j].down = -10000;
      }
      if (j==0) {
        Q[(i*6)+j].left = -10000;
      }
      if (j==5) {
        Q[(i*6)+j].right = -10000;
      }
    }
  }
  current_outer = 2;
  current_inner = 2;
  inner.write(2*25);
  outer.write(2*25);

  score = 1;
  
  Serial.begin(9600); // Starts the serial communication
  Serial.println(Q[15].up);
}



void loop() {
  // Do best action
  current_max = getMax(current_inner, current_outer);
  move(current_max.i, current_max.o);
  delay(200);
  pot_val = analogRead(A0);          //Read and save analog value from potentiometer
  pot_val = map(pot_val, 0, 1023, 0, 255);
  score += WALK_REWARD;
  reward = (pot_val - old_val)/100 + WALK_REWARD;
  old_val = pot_val;
  Serial.print("max q: ");
  Serial.println(current_max.q);
  
  Serial.print("Pot val: ");
  Serial.println(pot_val);
  Serial.print("Score: ");
  Serial.println(score);
  Serial.println("Q:");
  for (int i = 0; i < 6; i++){
    for (int j = 0; j < 6; j++) {
      Serial.print(Q[(i*6)+j].up);
      Serial.print(" ");
    }
    Serial.println("");
  }
  Serial.println("");
  for (int i = 0; i < 6; i++){
    for (int j = 0; j < 6; j++) {
      Serial.print(Q[(i*6)+j].down);
      Serial.print(" ");
    }
    Serial.println("");
  }
  Serial.println("");
  for (int i = 0; i < 6; i++){
    for (int j = 0; j < 6; j++) {
      Serial.print(Q[(i*6)+j].right);
      Serial.print(" ");
    }
    Serial.println("");
  }
  Serial.println("");
  for (int i = 0; i < 6; i++){
    for (int j = 0; j < 6; j++) {
      Serial.print(Q[(i*6)+j].left);
      Serial.print(" ");
    }
    Serial.println("");
  }

  // Update Q
  current_max = getMax(current_inner, current_outer);
  update_Q();
  delayMicroseconds(20);
  
  
  // 55
  // 66
  

}
