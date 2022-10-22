#include "jimlib.h"


#include "AsyncUDP.h"
AsyncUDP udp;

JStuff j;

struct {
	int pwm = 16;
} pins;


Timer minute(60000), blink(100), second(1000);


CLI_VARIABLE_FLOAT(pwm, 1000);
CLI_VARIABLE_FLOAT(lo, 300);
CLI_VARIABLE_FLOAT(hi, 1700);
CLI_VARIABLE_FLOAT(led, 1);
CLI_VARIABLE_STRING(test, "test");

void setup() {
	j.mqtt.active = false;

	j.begin();
	j.cli.on("MINUTE", [](){ minute.alarmNow(); });
	ledcSetup(0, 50, 16); // channel 1, 50 Hz, 16-bit width
	ledcAttachPin(pins.pwm, 0);   // GPIO 33 assigned to channel 1

	j.onConn = []{
		if (udp.listen(1234)) { 
			udp.onPacket([](AsyncUDPPacket packet) {
				String s = buf2str(packet.data(), packet.length());
				
				int c = 0;
				if (sscanf(s.c_str(), "set led=%d", &c)) {
					j.led.setPercent(c);
				}
				if (sscanf(s.c_str(), "set pwm=%d", &c)) {
					if (c >= lo && c <= hi) {
						ledcWrite(0, c * 4915 / 1000);
					}
				}

				//j.cli.process(s.c_str());
				//led = led != 0 ? 0 : 100;
				OUT(s.c_str());
			});
		}
	};
}

void loop() {
	j.run();
	delayMicroseconds(10);
}

