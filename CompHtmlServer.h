#pragma once
#include "HttpServer.h"

// Server
WiFiServer wifi;
HttpServer server(wifi);
const char *ssid = "YOUR_SSID";
const char *password = "YOUR_PW";

static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html>
    <head>
        <title>Compressor</title>
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <style>
            body { background-color: #cccccc; font-family: Arial; text-align: left; margin: 0px auto; padding-top: 20px; padding-left: 30px;}
            .slider { width: 90%; }
        </style>
        <script src = 'https://ajax.googleapis.com/ajax/libs/jquery/2.1.3/jquery.min.js'></script>
        <script>
            const serive_url = '/service';
            $(document).ready(function() {
	            // load values
                $.getJSON(serive_url, function(data) {
                    for(var effect in data){
                        id = "#"+effect
                        $(id).attr({
                            "max" : data[effect].max,
                            "min" : data[effect].min,
                            "value": data[effect].value,
                            "step" : data[effect].step
                        });
                    }
                });
	            // submit form
              $('#effect-form').submit(function( event ) {
		        	  event.preventDefault();
    				    const data = new FormData(event.target);
    				    const value = Object.fromEntries(data.entries());
    				    const json = JSON.stringify(value);
		            // send ajax
		            $.ajax({
						      url: serive_url,
						      type: 'POST',
						      dataType: 'json',
						      data: json,
						      contentType: 'text/json',
						      success : function(result) {console.log(json);},
						      error: function(xhr, resp, text) {
							       console.log(json, xhr, resp, text);
						      }
					       });
		            });
              });  // <label for='RatioControl'>Ratio 0-100%</label>
        </script>
    </head>
    <body>
        <h1>Compressor</h1>
        <form id="effect-form" method='post' >
            <div>
                Ratio 10-100%<br>
                <input type='range' class="slider" id='RatioControl' name='RatioControl' onchange="$('#effect-form').submit();" min='0' max='0' step='0' value='0' >
            </div>
            <div>
                <br>Threshold 10-100%<br>
                <input type='range' class="slider" id='Threshold' name='Threshold' onchange="$('#effect-form').submit();" min='0' max='0' step='0' value='0'>
            </div>
            <div>
                <br>Attack 5-100ms<br>
                <input type='range' class="slider" id='AttackTime' name='AttackTime' onchange="$('#effect-form').submit();" min='0' max='0' step='0' value='0'>
            </div>
            <div>
                <br>Release 10-1000ms<br>
                <input type='range' class="slider" id='ReleaseTime' name='ReleaseTime' onchange="$('#effect-form').submit();" min='0' max='0' step='0' value='0'>
            </div>
        </form>
    </body>
</html>
)rawliteral";


void getHtml(HttpServer *server, const char*requestPath, HttpRequestHandlerLine *hl) { 
    server->reply("text/html", (const char *)INDEX_HTML, 200);
};


