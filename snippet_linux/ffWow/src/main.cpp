//
// export LD_LIBRARY_PATH="/media/palette/thirdparty/lib"
//

#include <stdio.h>

#include <app.h>

#define		MAX_FF_EFFECTS		(3)

#define		FF_WEAK_SINE		(0)
#define		FF_NORMAL_SINE		(1)
#define		FF_HEAVY_SINE		(2)

SDL_Haptic*		pHaptic = NULL;
SDL_Joystick*	pJoystick = NULL;
int 			nEffectDuration = 1000;	

typedef struct 
{
	int					idEffect;
	SDL_HapticEffect	effect;
}stEffectProperty;

stEffectProperty	arrEffects[MAX_FF_EFFECTS];

void loadHapticEffects()
{
	SDL_HapticEffect	effect;
	int index;
	
	memset( &effect, 0, sizeof(SDL_HapticEffect) ); 	
	effect.type = SDL_HAPTIC_SINE;
	effect.periodic.direction.type = SDL_HAPTIC_POLAR; 	
	effect.periodic.direction.dir[0] = 18000; 			
	effect.periodic.period = 1000; 	
	effect.periodic.magnitude = 2500; 					// 20000/32767 strength
	effect.periodic.length = nEffectDuration; 			
	effect.periodic.attack_length = 100; 	
	effect.periodic.fade_length = 100; 
	arrEffects[FF_WEAK_SINE].effect = effect;

	memset( &effect, 0, sizeof(SDL_HapticEffect) ); 	
	effect.type = SDL_HAPTIC_SINE;
	effect.periodic.direction.type = SDL_HAPTIC_POLAR; 	
	effect.periodic.direction.dir[0] = 18000; 			
	effect.periodic.period = 1000; 	
	effect.periodic.magnitude = 5000; 					// 20000/32767 strength
	effect.periodic.length = nEffectDuration; 			
	effect.periodic.attack_length = 100; 	
	effect.periodic.fade_length = 100; 
	arrEffects[FF_NORMAL_SINE].effect = effect;

	memset( &effect, 0, sizeof(SDL_HapticEffect) ); 	
	effect.type = SDL_HAPTIC_SINE;
	effect.periodic.direction.type = SDL_HAPTIC_POLAR; 	
	effect.periodic.direction.dir[0] = 18000; 			
	effect.periodic.period = 1000; 	
	effect.periodic.magnitude = 10000; 					// 20000/32767 strength
	effect.periodic.length = nEffectDuration; 			
	effect.periodic.attack_length = 100; 	
	effect.periodic.fade_length = 100; 
	arrEffects[FF_HEAVY_SINE].effect = effect;

	printf("Checking ForceFeedback Hardware\n");
	for(index=0; index < MAX_FF_EFFECTS; index++)
	{
		if (0 == ( SDL_HapticQuery(pHaptic) & (arrEffects[index].effect.type) )) 
		{
			printf("Skipping Haptic Effect %d\n",index);
		}
		else
		{
			printf("Uploading Haptic Effect ",index);
			arrEffects[index].idEffect = SDL_HapticNewEffect( pHaptic, &arrEffects[index].effect);
			printf(":: Assigned %d\n",arrEffects[index].idEffect);
		}
	}
}

void playHapticEffect(int nEffectIndex, int nWaitDuration)
{
	SDL_HapticRunEffect(pHaptic, nEffectIndex, 1);
	SDL_Delay(nWaitDuration); // Wait for the effect to finish
}

void initialiseJoystick()
{
	int 		nNumDevices;
	
	nNumDevices = SDL_NumJoysticks();

	if(nNumDevices > 0)
	{
		printf("Found Joystick\n");

		pJoystick = SDL_JoystickOpen(0);
	}
	else
	{
		printf("No Joystick found\n");
	}
}

void initialiseHaptics()
{
	int 		nNumDevices;
	
	nNumDevices = SDL_NumHaptics();

	if(nNumDevices > 0)
	{
		printf("Found Haptic Device\n");

		pHaptic = SDL_HapticOpen(0);

		loadHapticEffects();
	}
	else
	{
		printf("No Haptic Devices found\n");
	}
}

void initialiseExperience()
{
	SDL_Init( SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC );
	
	initialiseJoystick();
	initialiseHaptics();
}

void closeHaptics()
{
	int index;
	
	if(pHaptic)
	{
		for(index=0; index < MAX_FF_EFFECTS; index++)
		{
			if(arrEffects[index].idEffect > 0)
			{
				SDL_HapticDestroyEffect( pHaptic, arrEffects[index].idEffect );
			}
		}

		SDL_HapticClose(pHaptic);
	}
}

void closeJoystick()
{
	if(pJoystick)
	{
		SDL_JoystickClose(pJoystick);
	}
}

void closeExperience()
{
	closeHaptics();
	closeJoystick();

	SDL_Quit();
}

void processEvents()
{
	int bQuit = 0;
	SDL_Event oEvent;

	do
	{
		if( SDL_WaitEvent( &oEvent ) )
		{
			switch(oEvent.type)
			{  
				// KB events from SDL window; terminal key presses are ignored
				case SDL_KEYDOWN:

					switch( oEvent.key.keysym.sym )
					{
						case SDLK_SPACE:
							bQuit = 1;
							break;

						default:
							break;
					}
					break;

			    case SDL_QUIT:
					bQuit = 1;
					break;

				case SDL_JOYAXISMOTION:
					if ( ( oEvent.jaxis.value < -3200 ) || (oEvent.jaxis.value > 3200 ) ) 
					{
						printf("Axis %d : Value %d\n", oEvent.jaxis.axis, oEvent.jaxis.value);
					}
					break;

				case SDL_JOYBUTTONDOWN:
					printf("Joystick Button %d\n", oEvent.jbutton.button);
					
					switch(oEvent.jbutton.button)
					{
						case 5:
							playHapticEffect(arrEffects[FF_HEAVY_SINE].idEffect, nEffectDuration);
							break;

						case 7:
							playHapticEffect(arrEffects[FF_NORMAL_SINE].idEffect, nEffectDuration);
							break;

						case 4:
							playHapticEffect(arrEffects[FF_WEAK_SINE].idEffect, nEffectDuration);
							break;

						case 3:
							bQuit = 1;
							break;

						default:
							break;
					}
					break;

				case SDL_JOYHATMOTION:
					printf("Joystick Hat %d\n", oEvent.jhat.value);
					break;

				default:
					break;
			}
		}

	}while(!bQuit);
}

int main(void)
{
	printf("Initialising experience\n");
	initialiseExperience();

	processEvents();

	printf("\n");
	printf("Closing experience\n");
	closeExperience();

	return 0;
}

