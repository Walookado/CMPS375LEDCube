
//intializing and declaring layers
int layer[2]={A2,A3};
//initializing and declaring leds, bottom right is led[0]
int led[4]={0,1,4,5};
int time = 100;
void setup() {
	// put your setup code here, to run once:
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

void loop() {
	// put your main code here, to run repeatedly:
	//Currently set to blink on and off every second.
	//turnOn();
	blink();
	
}

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

void turnOn()
{
	for(int i = 0; i<4; i++)
	{
		digitalWrite(led[i], 1);
	}
	//turning on layers
	for(int i = 0; i<2; i++)
	{
		digitalWrite(layer[i], 0);
	}
}

void blink()
{
    int pin;
    bool looping = true;
    int ledVal = 0;
    int layerVal = 1;
    
    // LED active on (HIGH, 1), layer active on (LOW, 0)
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
	}
}