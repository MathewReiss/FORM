Pebble.addEventListener("showConfiguration", function(){
	Pebble.openURL("https://dl.dropboxusercontent.com/u/17829339/form.html");
});

Pebble.addEventListener("webviewclosed", function(e){
	var options = JSON.parse(decodeURIComponent(e.response));
	var theme = options.theme;
	var dict = {"THEME" : theme};
	Pebble.sendAppMessage(dict);
});