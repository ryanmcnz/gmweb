//Audio stuff
var sfxButton;
var bgmMenu = "http://www.youtube.com/embed/4HuPUxRN-g4?html5=1&rel=0&autohide=1&showinfo=0&loop=1&autoplay=1&playlist=4HuPUxRN-g4";

//Performs a CSS transition between GM rooms. This one is 3 seconds long.
function roomTransitionAnimate(){
    var skipAnimation = (window.location.hash == "#no_transition");
    if(!skipAnimation){
	   //Animate title card bg
	   $("#room_transition").addClass("animating");
	   //Animate title card fg
	   $("#room_title").addClass("animating");
    }

	//Change the game room while the title card is fully covering the screen
	setTimeout(function(){
		changeRoom("rmMainMenu");
		$("#main_menu").fadeIn(skipAnimation ? 160 : 2000);
	},skipAnimation ? 5 : 1500);

	//Reset title card animation states
	setTimeout(function(){
		$("#room_transition").removeClass("animating");
		$("#room_title").removeClass("animating");
		//Play some music as well
		//$("#bgm_youtube").attr("src",bgmMenu);
	},skipAnimation ? 5 : 3000);
    
    if(skipAnimation){
        lowLag.play("sfxNavBack");
    }
}

$(document).ready(function(){
	$("#main_menu button").on("mouseover",function(el){
		//sfxButton.play();
		lowLag.play("sfxButton");
		//Update and show description for button
		$("#main_menu_desc").html( $(el.target).attr("desc") );
		Cufon.replace("#main_menu_desc", {color: '#000'});
		$("#main_menu_desc").show();
	});
	$("#main_menu button").on("mouseout",function(el){
		//Hide button description
		$("#main_menu_desc").hide();
		$("#main_menu_desc").html("");
	});
	//When a demo is selected on the menu, animate a transition to that room.
	$("#main_menu button").on("click",function(el){
		lowLag.play("sfxNavFwd");
		//Hide the menu
		$("#main_menu").fadeOut(200);
		//Fade everything out to blue
		//$("body").addClass("main_menu_end");
		//Change the room in GM, once the transition has happened
		$("#main_menu_desc").hide();
		setTimeout(function(){
			changeRoom( "rmDemo" + ( $(el.target).index() + 1) );
		},260);
	});

	//#main_menu_desc

	Cufon.replace("#room_title .txt"
		,{
			color: '-linear-gradient(#ddd, 0.45=#fff, 0.45=#ddd, #fff)'
			,textShadow: '4px 4px 0px #000'
		}
	);

	Cufon.replace("#main_menu .title"
		,{
			color: '-linear-gradient(#f39c12, #e74c3c)'
			,textShadow: '6px 5px 0px #000'
		}
	);

	Cufon.replace("#main_menu .sub_title"
		,{
			color: '-linear-gradient(#eeeeee, #888888)'
			,textShadow: '1px 2px 0px #000'
		}
	);

	Cufon.replace("#main_menu button"
		,{
			color: '-linear-gradient(#faecab, #ffb656)'
			,textShadow: '2px 3px #000'
		}
	);

	//DEBUG: This delay is just so we can see the background image in the first GM room, "rmLoading"
	setTimeout(function(){
		roomTransitionAnimate();
	},700);
	// },1);

});