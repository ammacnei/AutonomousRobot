#pragma config(Sensor, in1,    IRSensor,       sensorReflection)
#pragma config(Sensor, in8,    armposition,    sensorPotentiometer)
#pragma config(Sensor, dgtl4,  LED1,           sensorLEDtoVCC)
#pragma config(Sensor, dgtl6,  LED2,           sensorLEDtoVCC)
#pragma config(Sensor, dgtl10, startbutton,    sensorTouch)
#pragma config(Sensor, dgtl11, sonarIn,        sensorSONAR_cm)
#pragma config(Motor,  port1,           arm,           tmotorVex393_HBridge, openLoop)
#pragma config(Motor,  port2,           wheelleft,     tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port3,           wheelright,    tmotorVex393_MC29, openLoop)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

/*
	Final Milestone: Code Generated, Tested and Edited by ENGR 120/121 students, in Lab Section B11, Group 058
	University of Victoria
	Department of Engineering
	March 17th, 2016
	Student Names and Numbers:
	Amber MacNeill - V00827818
	Isabelle Weber-Concannon - V00797980
	Aren Beagley - V00851240
*/

bool startbutton_pushed; //store button input

/*
These are the global variables
*/
const int IR_FAR_AWAY = 75; //Infrared sensor value when far away from beacon
const int IR_MEDIUM_RANGE = 180; //larger value for infrared sensor when closer to beacon
const int IR_VERY_CLOSE = 3812; //even larger value for infrared sensor when really close to beacon
const int DROP_DISTANCE = 18; //distance sonar sensor reads when ready to drop object
const int TIME_TURN = 4400; //time for one complete turn with the wait 400 ms taken into account
const int PARALLEL = 1700; //position that arm is parallel to groun
const int TIME_REVERSE = 2500; //time to reverse after dropping object
const int TIME_COMPLETE = 3000; //time used to signal completion
const int ARM_LIFT = 1300; //position to lift arm to after drop
const int QUICK_TURN = 4400; //time for turn during check location


/*
 monitorInput was taken and adjusted from ProgrammingLab1.c file
 this function is used to monitor if the start button was pushed.
*/
void monitorInput(){
	if(SensorValue(startbutton) && !startbutton_pushed){
  	startbutton_pushed = true; //when startbutton is pushed, set bool to true
  }//end if

}//end monitor input

/*
This function samples photo-transistor circuit and returns integer value
*/
int SampleIR(){
	int noise = 4096;
	int highestValue = 0;
	int IRInput;
	clearTimer(T1); //different timer used that in other functions to prevent a logic error
	while(time1[T1] < 105){
  	IRInput = SensorValue[IRSensor];
  	if(IRInput > highestValue){ //if the IRsensor reads a higher value than current value, store that value as highest IR
   		highestValue = IRInput;
  	}//end if
		if(IRInput < noise){ //if IRsensor reads a value that is less than the noise, store that value as noise
 	 		noise = IRInput;
   	}//end if
	}//end while
	return (highestValue - noise); //returns integer value of the sample
} //end sample IR

/*
This function checks the position of the robot before approaching beacon
It moves towards the farthest wall so that it doesn't run into the wall on its approach towards the beacon
*/
void checkLocation(){
	int wall;
	clearTimer(T2);
	while(time1[T2] < QUICK_TURN){ //turn while reading sonar
		motor[wheelleft] = 20;
		motor[wheelright] = -22;
		wall = SensorValue[sonarIn];
		if((wall > 240) && (wall < 300)){ //if sonar reads between 240 and 300 move towards that wall for two seconds
			clearTimer(T1);
			motor[wheelleft] = 0;
			motor[wheelright] = 0;
			clearTimer(T3);
			wait1Msec(100);
			motor[wheelleft] = 30;
			motor[wheelright] = 30;
			while(time1[T1] < 2000){
				if(SensorValue[sonarIn] < 70) { //if robot gets too close to a wall re-check location
					checkLocation();
					return;
				}//end if
			}//end while
			break;
		}//end if
	}//end while
}//end check location

/*
 This function is used to estimate the direction of the beacon with a given IRValue
*/
void findBeacon(int IRValue){
	while(SampleIR() < IRValue){ //turn right until beacon is found
  	motor[wheelleft] = 20;
  	motor[wheelright] = -20;
 	}//end while
	motor[wheelleft] = 0; //stop turning when beacon is found
	motor[wheelright] = 0; //stop turning when beacon is found
}//end find beacon

/*
This functions finds the robot, approaches the robot, and stops when the robot is ready to drop the object
During the robots appraoch, it purposely veers to the left and re-finds the target
This veering and re-finding allows the robot to approach the beacon without needing to approach straight on from far away.
It makes it's approach more accurate.
*/
void findApproach(){
	findBeacon(IR_FAR_AWAY); //Call to find beacon using a global variable
 	clearTimer(T2);
 	bool isCloseToBeacon = false; //sets is close to beacon to false.
 	while(time1[T2] < 1900){//approach beacon intentionally veering to the left
 		motor[wheelleft] = 18;
  	motor[wheelright] = 25;
 	}//end while
	while(!isCloseToBeacon){ //find the beacon when the robot isn't too close
  	findBeacon(IR_MEDIUM_RANGE); //call to find beacon using a global variable
  	clearTimer(T2);
  	if(SensorValue[sonarIn] <35){ //checks to see if robot is close to beacon
  		isCloseToBeacon = true; //sets bool to true to signify that robot is close to beacon
  	}
  	while((time1[T2] < 900) && (!isCloseToBeacon)){ //veers left intentionally for almost 1 second
 			motor[wheelleft] = 18;
 			motor[wheelright] = 23;
 		}//end while
  //end if
	}//end while
 	findBeacon(IR_VERY_CLOSE); //finds beacon when robot is close
 	while((SensorValue[sonarIn] > DROP_DISTANCE)){ //approaches robot until drop distance
 		motor[wheelleft] = 18;
  	motor[wheelright] = 23; //corrects for weak right wheel motor
	}//end while
	motor[wheelleft] = 0; //stop in front of beacon
 	motor[wheelright] = 0;
}//end findApproach

/*
This function is used to drop the object onto the beacon and move away
*/
void dropObject(){
	clearTimer(T1);
	while(SensorValue[armposition]  < PARALLEL){ //lower object for drop
		motor[arm] = -20;
  }//end while
	motor[arm] = 0;
 	clearTimer(T1);
 	while(time1[T1] < TIME_REVERSE ){ //move backward away from beacon
 		motor[wheelright] = -25;
 		motor[wheelleft] = -23;
 	}//end while
 	motor[wheelleft] = 0; //stop robot
 	motor[wheelright] = 0;
 	clearTimer(T1);
 	while(SensorValue[armposition] > ARM_LIFT){ //raise arm
 		motor[arm] = 30;
 	}//end while
 	motor[arm] = 0;
}//end dropObject

/*
This function is used to detect the edge of the arena
*/
void detectEdge(){
	int edge = 1000;
 	motor[wheelleft] = 22; //turn right
 	motor[wheelright] = -20;
 	int distance;
 	wait1Msec(400);
 	clearTimer(T2);
 	while(time1[T2] < TIME_TURN){ //detect the closest wall to the robot
 		distance = SensorValue[sonarIn]; //set distance to sonar value
  	if(distance < edge){ //sets closest wall to edge to go towards
   		if(SampleIR() < 20){ //check to see that the edge isn't the beacon
   			edge = distance; //set the closest wall
 		 	} //end if
 		}// end if
 	}// end while

 	int dist = 1001;
 	bool edgeFound = false;
 	int fullRotations = 0;
 	int uncertainty = 3;
 	clearTimer(T2);
 	while(!edgeFound){ //leaves this loop when edge to approach is found
 		dist = SensorValue[sonarIn];
 		/*	if distance is less than edge + uncertainty
 				and distance is greater than edge - 1
 				it is pointing at the closest wall and not the beacon
 				so break the while loop
 		*/
 		if((dist < (edge + uncertainty)) && (dist > (edge - 1))){
 				edgeFound = true;
 		}
 		else if(time1[T2] > TIME_TURN){ //if edge is not found increase uncertainty and rotations counter
 			uncertainty = uncertainty + 1 + fullRotations;
 			clearTimer(T2);
 			fullRotations++;
 		}//end else if
 		if(fullRotations >= 1) { //if edge is still not found move forward for half a second not towards beacon and then re-detect edge
 			clearTimer(T2);
 			while((time1[T2] < 500) && SampleIR() < 50){
 				motor[wheelright] = 30;
 				motor[wheelleft] = 30;
 			}//end while
 			detectEdge();
 			return;
		}//end if
 	}//end while
	while(SensorValue[sonarIn] > 6){ //keep moving forward until sonar read 6 cm (6 to correct for momentum of moving forward)
 		motor[wheelleft] = 20; //approach wall
 		motor[wheelright] = 20;
  }//end while
 	motor[wheelleft] = 0;
 	motor[wheelright] = 0;
}// end detect edge

/*
This function is used signal completion of task
Turns on LEDs and moves drop arm up and down
*/
void signalCompletion(){
	SensorValue(LED1) = 0;
  SensorValue(LED2) = 1;
	clearTimer(T1);
	while(time1[T1] < TIME_COMPLETE){
		if(SensorValue[armposition] < (PARALLEL-200)){
			motor[arm] = -25;
		}
		else if(SensorValue[armposition] >= PARALLEL){
			motor[arm] = 25;
		}//end if
	}//end while
	motor[arm] = 0;
}//end signalCompletion

task main()
{
	while(1){
		monitorInput();
		SensorValue(LED1) = 1; //LED1 is off (different off values)
    SensorValue(LED2) = 0; //LED2 is off (different off values)
		if(startbutton_pushed){
     	checkLocation();
			findApproach();
     	wait1Msec(200); //wait 200 milliseconds
  		dropObject(); //drop object, and move away from beacon
  		detectEdge();
  		signalCompletion();
  		startbutton_pushed = false;
		}//end if
	}//end while

}//end task main
