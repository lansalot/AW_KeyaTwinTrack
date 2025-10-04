
void motorDrive(void) {
	if (keyaDetected)
	{
		SteerKeya(keyaIntendToSteer);
	}
	pwmDisplay = 0;
}
