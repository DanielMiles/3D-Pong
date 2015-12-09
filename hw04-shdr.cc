/// LSU EE 4702-1 (Fall 2015), GPU Programming
//
 /// Homework 4
 //
 //  Use this file for your solution.

 /// Instructions
 //
 //  Read the assignment: http://www.ece.lsu.edu/koppel/gpup/2015/hw04.pdf



// Specify version of OpenGL Shading Language.
//
#version 430 compatibility

layout ( location = 2 ) uniform int tri_cull;

// Use this variable to debug your code. Press 'y' to toggle tryout.x
// and 'Y' to toggle debug_bool.y (between true and false).
//
layout ( location = 3 ) uniform bvec2 tryout;



// Array of ball positions.
//
layout ( binding = 1 ) buffer Balls_Pos { vec4 balls_pos_rad[]; };
layout ( binding = 2 ) buffer Balls_Color { vec4 balls_color[]; };
layout ( binding = 3 ) buffer Paddle_Centers { vec4 paddle_centers[]; };
layout ( binding = 4 ) buffer Glow_Colors { vec4 glow_colors[]; };


#ifdef _VERTEX_SHADER_

// Interface block for vertex shader output / geometry shader input.
//
out Data_to_GS
{
  vec3 normal_e;
  vec4 vertex_e;
  vec4 color;
  vec4 gl_Position;
  vec4 paddle_center_0;
  vec4 paddle_center_1;
  vec4 glow_color_0;
  vec4 glow_color_1;

  // Any changes here must also be made to the fragment shader input.

};


void
vs_main()
{
  // Here, the vertex shader does nothing except pass variables
  // to the geometry shader.

  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
  vertex_e = gl_ModelViewMatrix * gl_Vertex;
  normal_e = gl_NormalMatrix * gl_Vertex.xyz;
  color = gl_Color;
}

void
vs_main_instances()
{
  vec4 center_o_rad = balls_pos_rad[gl_InstanceID];
  float rad = center_o_rad.w;
  vec3 to_surface = rad * gl_Vertex.xyz;
  vec4 position_o = vec4(center_o_rad.xyz + to_surface, 1f);

  gl_Position = gl_ModelViewProjectionMatrix * position_o;
  vertex_e = gl_ModelViewMatrix * position_o;
  normal_e = gl_NormalMatrix * gl_Vertex.xyz;
  color = balls_color[gl_InstanceID];
}

#endif


#ifdef _GEOMETRY_SHADER_

in Data_to_GS
{
  vec3 normal_e;
  vec4 vertex_e;
  vec4 color;
  vec4 gl_Position;
  vec4 paddle_center_0;
  vec4 paddle_center_1;
  vec4 glow_color_0;
  vec4 glow_color_1;
} In[];

out Data_to_FS
{
  vec3 normal_e;
  vec4 vertex_e;
  flat vec4 color;
  vec4 paddle_center_0;
  vec4 paddle_center_1;
  vec4 glow_color_0;
  vec4 glow_color_1;
};

// Type of primitive at geometry shader input.
//
layout ( triangles ) in;

// Type of primitives emitted geometry shader output.
//
layout ( triangle_strip, max_vertices = 3 ) out;

void
gs_main_simple()
{
  /// Problem 1 and 3 Solution Here.
  //
  //  Behavior:
  //    When try_cull = 0, emit all triangles.
  //    When try_cull = 1, only emit triangles facing user.
  //    When try_cull = 2, only emit triangles not facing user.

  for ( int i=0; i<3; i++ )
    {
      normal_e = In[i].normal_e;
      vertex_e = In[i].vertex_e;
      color = In[i].color;
      gl_Position = In[i].gl_Position;
      EmitVertex();
    }
  EndPrimitive();
}



#endif


#ifdef _FRAGMENT_SHADER_

in Data_to_FS
{
  vec3 normal_e;
  vec4 vertex_e;
  flat vec4 color;
  vec4 paddle_center_0;
  vec4 paddle_center_1;
  vec4 glow_color_0;
  vec4 glow_color_1;

};

vec4 generic_lighting(vec4 vertex_e, vec4 color, vec3 normal_e);
vec4 emissive_lighting(vec4 vertex_e, vec4 color, vec3 normal_e);
vec4 new_lighting(vec4 vertex_e, vec4 color, vec3 normal_e);
vec4 new_lighting2(vec4 vertex_e, vec4 color, vec3 normal_e);

void
fs_main()
{
  // Perform lighting, fetch and blend texture, then emit fragment.
  //

  // Multiply filtered texel color with lighted color of fragment.
  //
  //gl_FragColor = generic_lighting(vertex_e, color, normalize(normal_e));
  //gl_FragColor = new_lighting(vertex_e, color, normalize(normal_e));
  gl_FragColor = new_lighting2(vertex_e, color, normalize(normal_e));
  //gl_FragColor = glow_color_0;
  //gl_FragColor = emissive_lighting(vertex_e, gl_FragColor, normalize(normal_e));
  // Copy fragment depth unmodified.
 
  gl_FragDepth = gl_FragCoord.z;
}


///
/// Routine used by Either Vertex or Fragment Shader
///

vec4
generic_lighting(vec4 vertex_e, vec4 color, vec3 normal_e)
{
  // Return lighted color of vertex_e.
  //
  vec4 light_pos = gl_LightSource[0].position;
  vec3 v_vtx_light = light_pos.xyz - vertex_e.xyz;
  float d_n_ve = -dot(normal_e,vertex_e.xyz);
  float d_n_vl = dot(normal_e, normalize(v_vtx_light).xyz);
  bool same_sign = ( d_n_ve > 0 ) == ( d_n_vl > 0 );
  float phase_light = same_sign ? abs(d_n_vl) : 0;

  vec3 ambient_light = gl_LightSource[0].ambient.rgb;
  vec3 diffuse_light = gl_LightSource[0].diffuse.rgb;
  float dist = length(v_vtx_light);
  float distsq = dist * dist;
  float atten_inv =
    gl_LightSource[0].constantAttenuation +
    gl_LightSource[0].linearAttenuation * dist +
    gl_LightSource[0].quadraticAttenuation * distsq;
  vec4 lighted_color;
  lighted_color.rgb =
    color.rgb * gl_LightModel.ambient.rgb
    + color.rgb * ( ambient_light + phase_light * diffuse_light ) / atten_inv;
  lighted_color.a = color.a;
  return lighted_color;
}

vec4
new_lighting(vec4 vertex_e, vec4 color, vec3 normal_e)
{
  // Return lighted color of vertex_e.
  //
  vec4 light_position = gl_LightSource[0].position;
  float dist = distance(vertex_e,light_position);
  float att=1.0/(1.0+0.1*dist+0.01*dist*dist);
  vec4 surf2light = normalize(light_position - vertex_e);
  vec4 norm = vec4(normal_e, 1.0);

  vec4 light_color = vec4(gl_LightSource[0].diffuse.rgb * att,1.0);

  return color * (0.1+light_color);
}

vec4
new_lighting2(vec4 vertex_e, vec4 color, vec3 normal_e)
{
  // Return lighted color of vertex_e.
  //
  vec4 light_position = gl_LightSource[0].position;
  vec4 Kd = vec4(gl_LightSource[0].diffuse.rgb,1);
  vec4 Ld = vec4(255,255,255,1);
  
  vec4 s = normalize(vec4(light_position-vertex_e));
  vec4 intensity = Ld*Kd*max(dot(s,vec4(normal_e,1.0)),0.0);
  
  return color * intensity;
}

vec4
emissive_lighting(vec4 vertex_e, vec4 color, vec3 normal_e)
{
  vec4 paddle_center_0_e = gl_ModelViewMatrix * paddle_center_0;
  vec4 dist = paddle_center_0_e - vertex_e;
  float d = sqrt(dist.x*dist.x + dist.y*dist.y + dist.z*dist.z);
  float d_max = 1.5;
  d = min(d,d_max);
  vec4 white = {255,255,255,255};
  color = color*(d/d_max) + white*(1-d/d_max);
  
  return color;
}

#endif
