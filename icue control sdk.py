from cuesdk import CueSdk
import colorsys
import threading
import queue
import time
import requests
import threading
import httplib2
import time
from http.server import ThreadingHTTPServer, BaseHTTPRequestHandler
import ssl, threading

def getRGB(Hex):
    print(Hex)
    return tuple(int(str(Hex)[i:i+2], 16) for i in (0, 2, 4))

class CUSTOMHANDLER(BaseHTTPRequestHandler):
    def _set_response(self):
        self.send_response(200)
        self.send_header('Content-type', 'text/html')
        self.end_headers()
        
    def do_POST(self):
        global color
        global rainbow
        content_length = int(self.headers['Content-Length']) 
        post_data = self.rfile.read(content_length)
        if post_data.decode() == "rainbow":
            rainbow=True
            print("Turned on rainbow")
        else:
            print(post_data.decode())
            color = ("0"*(6-len(post_data.decode())))+post_data.decode()
            print(color)

            print('RGB =', getRGB(color))
        
        self._set_response()
        
server_address = ('', 16000)
httpd = ThreadingHTTPServer(server_address, CUSTOMHANDLER)
x = threading.Thread(target = httpd.serve_forever)
x.start()

def read_keys(inputQueue):
    while True:
         input_str = input()
         inputQueue.put(input_str)


def get_available_leds():
    leds = list()
    device_count = sdk.get_device_count()
    for device_index in range(device_count):
        led_positions = sdk.get_led_positions_by_device_index(device_index)
        leds.append(led_positions)
    return leds

rainbowColor = 0


def static(all_leds, rainbowColor):
    cnt = len(all_leds)
    
    for di in range(cnt):
        device_leds = all_leds[di]
        for led in device_leds:
            device_leds[led] = getRGB(("0"*(6-len(str(rainbowColor))))+str(rainbowColor))

        sdk.set_led_colors_buffer_by_device_index(di, device_leds)
    sdk.set_led_colors_flush_buffer()
    #print([round(i*255) for i in colorsys.hsv_to_rgb(rainbowColor/100, 1, 1)])
    #print(rainbowColor)
def rainbowEffect(all_leds, rainbowColor):
    cnt = len(all_leds)

    for di in range(cnt):
        device_leds = all_leds[di]
        for led in device_leds:
            device_leds[led] = [round(i*255) for i in colorsys.hsv_to_rgb(rainbowColor/100, 1, 1)]

        sdk.set_led_colors_buffer_by_device_index(di, device_leds)
    sdk.set_led_colors_flush_buffer()
    #print([round(i*255) for i in colorsys.hsv_to_rgb(rainbowColor/100, 1, 1)])
    #print(rainbowColor)


rainbow = False
def main():
    global sdk
    global color
    color = "ff0000"
    rainbowColor = 0
    sdk = CueSdk()
    connected = sdk.connect()
    print(connected)
    if not connected:
        err = sdk.get_last_error()
        print("Handshake failed: %s" % err)
        return
    
    while True:
        if rainbow:
            rainbowEffect(get_available_leds(), rainbowColor)
            time.sleep(0.05)
            rainbowColor += 1
            #print(rainbowColor*2)
            if rainbowColor > 99:
                rainbowColor=0
        else:
            static(get_available_leds(),color)
            

if __name__ == "__main__":
    main()
