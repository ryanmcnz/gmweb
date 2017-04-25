//Set the URL of the web content for GMWeb. This uses an absolute path for DEBUG, and EXE program directory for RELEASE
default_url = "\www\uiMainMenu.html";
//NOTE: Change this when working with your game in DEBUG mode, so that it points to your projects debug files.
//debug_path = "C:\path\to\project\GMWeb\GMWeb.gmx\BUILD";  //TODO: Use this one when actually putting GMWeb on the marketplace
debug_path = "D:\Programming\tools\GM demos\GMWeb\GMWeb.gmx\BUILD";
if(debug_mode){
    //Debug
    default_url = debug_path + default_url;
}
else{
    //Release
    default_url = program_directory + default_url;
}

/*
    Check whether the web content folder is in the game directory. If it isn't, show an error and close the game.
    NOTE:
        When distributing web content with a game, you'll want to place it in the same folder as the game EXE. This has multiple benefits.
        It allows gamers to easily modify the content. It also bypasses the restrictions enforced by file IO being sandboxed in GameMaker (by design).
        To make development easier we can harness the two separate game builds, provided by the IDE:
        - DEBUG:
            The IDE runs the game from the temp directory. The game looks for web content in an absolute path, configured by the developer.
        - RELEASE:
            The game EXE is stored in the same folder as the web content. The game accesses this content using a non-sandboxed program directory.
            GameMaker only supports a non-sandboxed program directory when you generate a ZIP. To do this, go "File" -> "Create Application" -> "Save as type" -> "zip"...and then unzip it.
            Without this method, building the game for release will result in an EXE that is only aware of the temp directory. Not very user-friendly!
*/
if(! gmweb_file_exists(default_url)){
    if(debug_mode){
show_message("ERROR: The web folder was not found in the game directory. The GMWeb examples cannot run without this.
For info on how to resolve this, please see the Create event code in the object 'objWebController'.");
    }
    else{
        //Show a more user-friendly message when not in debug
        show_message("ERROR: The UI folder was not found in the game directory. The game cannot run without this.");
    }
    game_end();
}
//Format the final path to actually be a URL, rather than a directory
default_url = "file:///" + string_replace_all(default_url,"\","/");
