vec4 checker_color()
{
    bool cx = fract(gl_FragCoord.x / 20.0) < 0.5;
    bool cy = fract(gl_FragCoord.y / 20.0) < 0.5;
    return cx && !cy || !cx && cy
        ? vec4(0.6, 0.6, 0.6, 1.0)
        : vec4(0.5, 0.5, 0.5, 1.0);
}
