//Cufon defaults. We use Cufon to have fancy text with effects on it (wouldn't be needed if Awesomium used the latest Chromium).
var defaultFontFamily = "Vegur";
var defaultFontWeight = 500;

//Add a simple wrapper for GMWeb when debugging the UI outside of GameMaker
if(window.gmweb === undefined){
    window.gmweb = {
        send: function(from,message){
            console.log([from,message]);
        }
    };
}

function changeRoom(roomName){
	//TODO: Code this using async JS. We'll use it half way during a room transition.
	//gmweb.send("main_menu", { eventSource: "changeRoom", eventData: roomName });
    gmweb.send("changeRoom", roomName);
}

//Animates the hint arrows, showing which direction the event is going between GameMaker and Chromium. 0 for right, 1 for left.
function animateEventArrow(direction){
    var nudgeDir = direction ? "left" : "right";
    $(".event_" + nudgeDir).removeClass("nudged");
    setTimeout(function(){
    $(".event_" + nudgeDir).addClass("nudged");
    },1);
}
//Displays a message sent from GM to the browser
function logBallClick(content){
    animateEventArrow(1);
    $("#from_gm_out").append("<p>" + content + "</p>");
    $('#from_gm_out').scrollTop($('#from_gm_out')[0].scrollHeight);
    return 1;
}

//Init frameworks and resources used across all GMWeb demos. Audio, Cufon styles, etc.
$(document).ready(function(){
	//Sound effects
	lowLag.init();
	lowLag.load(["media/sfxButton.ogg"],"sfxButton");
	lowLag.load(["media/sfxNavFwd.ogg"],"sfxNavFwd");
    lowLag.load(["media/sfxNavBack.ogg"],"sfxNavBack");
    
    //Style our fancy text with gradients and shadows
    Cufon.replace("h2", {
        fontFamily: defaultFontFamily, fontWeight: defaultFontWeight
        ,color:         "-linear-gradient(#f39c12, #e74c3c)"
        ,textShadow:    "4px 3px 0px #000"
    });
    Cufon.replace("h3", {
        fontFamily: defaultFontFamily, fontWeight: defaultFontWeight
        ,color:         "-linear-gradient(#ffb656,#dd7904)"
        ,textShadow:    "2px 3px 0px #000"
    });
    
	Cufon.replace("button", {
        fontFamily: defaultFontFamily, fontWeight: defaultFontWeight
        ,color:         "-linear-gradient(#faecab, #ffb656)"
        ,textShadow:    "2px 3px #000"
    });
    
    //Handle toggling of playing state for media, allowing for button graphics
    $("video,audio").on("play stop pause suspend loadstart",function(el){
        if(el.target.paused){
            $(el.target).removeAttr("playing");
        }
        else{
            $(el.target).attr("playing","");
        }
    });
    
    //Click events for all demos
    $("#demo_back").on("click",function(el){
        lowLag.play("sfxNavFwd");
        $("body").fadeOut(160);
        setTimeout(function(){
            changeRoom("rmMainMenu");
        },260);
    });
    
    //Click events for individual demos
    
    //Demo 1
    $("#ball_spawner").on("click",function(el){
        animateEventArrow(0);
        gmweb.send("spawnBall", {});
    });
    
});