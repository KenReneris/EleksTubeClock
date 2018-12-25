doc_videos\PerodicDate.mp4?raw=true# EleksTubeClock

The <a href='https://www.banggood.com/EleksMaker-EleksTube-Bamboo-6-Bit-Kit-Time-Electronic-Glow-Tube-Clock-Time-Flies-Lapse-p-1297292.html'>EleksTube</a> clock from <a href='https://www.kickstarter.com/projects/938509544/elekstube-a-time-machine'>Kickstarter</a> brings RGB addressable LEDs in a Nixie tube format powered by a USB source. 
This project replaces the controller that comes with the clock with an <a href='https://www.esp8266.com'>ESP8266</a> that syncs the time to an NTP time source over wifi, and includes a small webserver for configuration.
<br>
<br>
For the clock to work it needs to be configued to connect to a WiFi hotspot. Once the device has an IP address there are a number of settings which are <b>configured via a few web pages</b> accessible on the device.  Such as 12 or 24 mode, top of hour effect, etc..  The device also sync the current time from the internet and option the timezone (or the timezone can be manually set).
<br><br>
### Some videos:

Video | Description
--- | --- 
<a href="doc_videos\TopOfHour.mp4?raw=true">Top Of Hour</a> | Default Top Of Hour Effect.  Highlight appears, holds for 2 seconds, then Blends Out.
<a href="doc_videos\PerodicDate.mp4?raw=true">Periodic Date</a> | Optional periodic display of the date.  This example shows the Cross Fade effect with hold set to 3.
<a href="doc_videos\Popup.mp4?raw=true">Popup URL</a> | Sends a request to the clock to dipslay a popup. In this case '123456' which a Blend In Out effect.
<a href="doc_videos\BlendInOut.mp4?raw=true">BlendInOut</a> | Another video of the above blend in out effect.
<a href="doc_videos\boot.mp4?raw=true">Power on</a> | Power on sequence. Clock is already configured for local WiFi access.

<br><br>
### Some Images:

Image | Description
--- | --- 
<a href="doc_images\esp8266_mounted.JPG">Mounted</a> | The EleksTube controller removed and ESP8266 mounted to the base of the EleksTube case.  Here's the <a href="doc_images\esp8266_back.JPG">back</a>.
<a href="doc_images\4pin_1p25mm.JPG">Connector</a> | The female end of the 1.25mm plug. Notice the order of the colors.  The +5 V is the pin closest to the back side of the clock.
<a href="doc_images\console.jpg">Console</a> | Somce misc commands available on the serial console.
<a href="doc_images\settings_a.jpg">Settings</a> | Root web page containing links to the common settings.
<a href="doc_images\settings_b.jpg">More Settings</a> | More settings.
<a href="doc_images\settings_c.jpg">Clock Settings</a> | Example settings page.
<a href="doc_images\settings_date.jpg">Date Settings</a> | Example of date settings page.
<a href="doc_images\settings_date_color.jpg">Color Picker</a> | Setting the color of the periodic date effect. Embeded cotnrol from <a href='https://tovic.github.io/color-picker'>Taufik</a>.

<br><br>
### Effect types:

Effect | Description
--- | --- 
`Disabled`        | Action/opertion not displayed.
`Appear`          | Color/values appear and disappear without any transition effect.
`Cross Fade`      | Show the color/values using a fade to/from black transistions. <a href="doc_videos\PerodicDate.mp4?raw=true">example</a>
`Blend In Out`    | Blend to/from color/values. <a href="doc_videos\BlendInOut.mp4?raw=true">example</a>
`Appear Blend Out` | Color/values appear and then blend out.
`Flash`           | Color/values flash (1 to 15 times)






    
