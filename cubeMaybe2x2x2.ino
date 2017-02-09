
//intializing and declaring layers
int layer[2]={A1,A0};
//initializing and declaring leds, bottom right is led[0]
int led[8]={5,4,3,2,1,0,A5,A4};
int time = 100;
void setup() {
  // put your setup code here, to run once:
   for(int i = 0; i<8; i++)
  {
    pinMode(led[i], OUTPUT);
  }
  //setting layers to output
   for(int i = 0; i<2; i++)
  {
    pinMode(layer[i], OUTPUT);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  turnOff();
  turnOn();

}

void turnOff()
 {
   for(int i = 0; i<8; i++)
   {
     digitalWrite(led[i], 1);
   }
   for(int i = 0; i<2; i++)
   {
     digitalWrite(layer[i], 0);
   }
 }

void turnOn()
{
  for(int i = 0; i<8; i++)
  {
    digitalWrite(led[i], 0);
  }
  //turning on layers
  for(int i = 0; i<2; i++)
  {
    digitalWrite(layer[i], 1);
  }
}
