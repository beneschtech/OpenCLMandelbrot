const char *_kernelSrc =
        "__kernel void mandelbrotPoint(__global const double *params,int w,int h, __global unsigned char *img)\n"
        "{\n"
        "   int offst = get_global_id(0);\n"
        "   int x = offst % w;\n"
        "   int y = offst / w;\n"
        "   double xc = params[0] + ((params[2] * x)/w);\n"
        "   double yc = params[1] + ((params[3] * y)/h);\n"
        "   double xt = 0, yt = 0, xtt;\n"
        "   int i;\n"
        "   for (i=0; i < 1024; i++)\n"
        "   {\n"
        "      xtt = xt*xt - yt*yt + xc;\n"
        "      yt = 2 * xt * yt + yc;\n"
        "      xt = xtt;\n"
        "      if (xt * xt * yt * yt > 16)\n"
        "      {\n"
        "         img[offst] = i % 255;\n"
        "         return;\n"
        "      }\n"
        "   }\n"
        "}";

void _warningSilencer()
{
    (void)_kernelSrc;
}
