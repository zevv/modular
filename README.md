
This repo contains the electronic design files and firmware for an
analog/digital hybrid modular synthesizer system. The synth will be built from
a collection of separate modules which can be connected (patched) like a
traditional modular synth. The modules contain a fast CPU and an audio codec
and perform all work in the digital domain. The same PCB design is used in all
modules, and the firmware will make each module behave as a different function
like oscillators, filters, wave shapers, delay, reverb, etc.

Some specs for the module so far:

* 204Mhz dual core ARM Cortex M4/M0
* 4 audio outputs 10v peak-to-peak at 10kΩ, 96kHz/24bit
* 4 audio inputs 10v peak-to-peak at 330Ω, 96kHz/24bit
* 8 generic inputs 10v peak-to-peak at 10kΩ, 12kHz/10 bit
* 12V power supply, internally split into +5/-5 volts 

My primary goal is to make a working setup for my personal use. However, if I'm
happy enough with the results and other people are interested, I might consider
manufacturing and selling modules at a small scale.

