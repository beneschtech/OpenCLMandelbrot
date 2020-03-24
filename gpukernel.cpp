const char *_kernelSrc =
        "__kernel void mandelbrotPoint(__global const float *params,int w,int h,unsigned int iterations, __global unsigned char *img)\n"
        "{\n"
        "   unsigned int offst = get_global_id(0);\n"
        "   int x = offst % w;\n"
        "   int y = offst / w;\n"
        "   float xc = params[0] + ((params[2] * x)/w);\n"
        "   float yc = params[1] + ((params[3] * y)/h);\n"
        "   float xt = 0, yt = 0, xtt;\n"
        "   unsigned int i;\n"
        "      xc = params[0] + ((params[2] * x)/w);\n"
        "      img[offst] = 0;\n"
        "      for (i=0; i < iterations; i++)\n"
        "      {\n"
        "         xtt = xt*xt - yt*yt + xc;\n"
        "         yt = 2 * xt * yt + yc;\n"
        "         xt = xtt;\n"
        "         if (xt * xt * yt * yt > 16)\n"
        "         {\n"
        "            img[offst] = i % 256;\n"
        "            return;\n"
        "         }\n"
        "      }\n"
        "}";

void _warningSilencer()
{
    (void)_kernelSrc;
}
