//Title: LED Cube 2x2x2
//Authors: Team Haytom: Zachary Smith, Ericka Henderson, Kaylea Stewart


//intializing and declaring layers by pin number
int layer[2]={A2,A3};

//initializing and declaring LEDs by pin number
int led[4]={0,1,4,5};
	
// LED active on (HIGH, 1), layer active on (LOW, 0)	
	
int time = 100;


//Pin setup for OUTPUT
void setup() 
{
	for(int i = 0; i<4; i++)
	{
		pinMode(led[i], OUTPUT);
	}
	//setting layers to output
	for(int i = 0; i<2; i++)
	{
		pinMode(layer[i], OUTPUT);
	}
}

//Main loop for the arduino to run through while active
void loop() 
{
    blink();
    delay(500);
    turnColumnsOff();
    delay(500);
    randomflick();
}

//Basic function to turn off all LEDs
void turnOff()
{
	for(int i = 0; i<4; i++)
	{
		digitalWrite(led[i], 0);
	}
	
	for(int i = 0; i<2; i++)
	{
		digitalWrite(layer[i], 1);
	}
}

//Basic function to turn on all LEDs
void turnOn()
{
	for(int i = 0; i<4; i++)
	{
		digitalWrite(led[i], 1);
	}
	
	for(int i = 0; i<2; i++)
	{
		digitalWrite(layer[i], 0);
	}
}

//Simple pattern that turns off columns of LEDs in sequence
void turnColumnsOff()
{
    int runs = 0;
    while(runs < 5)
    {
        turnOn();
        
        for(int i = 0; i<2; i++)
        {
            delay(500);
            digitalWrite(led[i], 0);
        }
		
		for(int i = 3; i>1; i--)
        {
            delay(500);
            digitalWrite(led[i], 0);
        }
        
        runs++;
    }
}

//Simple pattern that turns all LEDs on and off
void blink()
{
    int pin;
    bool looping = true;
    int ledVal = 0;
    int layerVal = 1;
    int runs = 0;
    
    while(looping)
    {
        for(pin = 0; pin < 4; pin++)
        {
            digitalWrite(led[pin], ledVal);
		}
        
        for(pin = 0; pin < 2; pin++)
        {
            digitalWrite(layer[pin], layerVal);
		}
		
        if(ledVal == 0)
        {
            ledVal = 1;
            layerVal = 0;
		}
        
        else if(ledVal == 1){
            ledVal = 0;
            layerVal = 1;
		}
		
        delay(1000);
        
        if(runs >= 5)
        {
            break;
        }
        else
        {
            runs++;
        }
	}

}

//Pattern that turns random LEDs on and off
void randomflick()
{
  for(int i = 0; i < 20; i++)
  {
      int rLayer = random(0,2);
      int rLed = random(0,4);
      
      digitalWrite(layer[rLayer], 1);
      digitalWrite(led[rLed], 0);
      
      delay(200);
      
      digitalWrite(layer[rLayer], 0);
      digitalWrite(led[rLed], 1);
      
      delay(200); 
  }
}