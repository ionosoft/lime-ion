package lime.graphics.opengl; #if !html5


class GLShader extends GLObject {
	
	
	private override function getType ():String {
		
		return "Shader";
		
	}
    
    
}


#else
typedef GLShader = js.html.webgl.Shader;
#end