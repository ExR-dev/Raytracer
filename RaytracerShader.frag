
/*==========================================*/
/*                                          */
/*                  UTILS                   */
/*                                          */
/*==========================================*/

uniform float rndSeed;


float RandNum( vec2 p )
{
  // We need irrationals for pseudo randomness.
  // Most (all?) known transcendental numbers will (generally) work.
  const vec2 r = vec2(
    23.1406926327792690,  // e^pi (Gelfond's constant)
     2.6651441426902251); // 2^sqrt(2) (Gelfond–Schneider constant)
  return fract( cos( mod( 123456789., 1e-7 + 256. * dot(p,r) ) ) );  
}

































/*==========================================*/
/*                                          */
/*                  SHAPES                  */
/*                                          */
/*==========================================*/






































/*==========================================*/
/*                                          */
/*                 RENDERING                */
/*                                          */
/*==========================================*/

























/*==========================================*/
/*                                          */
/*                   MAIN                   */
/*                                          */
/*==========================================*/


uniform sampler2D texture;

void main(void)
{
    // lookup the pixel in the texture
    vec4 pixel = texture2D(texture, vec2(gl_TexCoord[0].x, 1.0 - gl_TexCoord[0].y));

    // multiply it by the color
    gl_FragColor = gl_Color * pixel;
}