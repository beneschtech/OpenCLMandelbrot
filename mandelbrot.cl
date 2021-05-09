__kernel void mandelbrotPoint(__global const double *params,int w,int h,unsigned int iterations, __global unsigned char *img)
{
    unsigned int offst = get_global_id(0);
    int x = offst % w;
    int y = offst / w;
    double xc = params[0] + ((params[2] * x)/w);
    double yc = params[1] + ((params[3] * y)/h);
    double xt = 0, yt = 0, xtt,ytt;
    unsigned int i;
    img[offst] = 0;
    for (i=0; i < iterations; i++)
    {
        xtt = xt*xt - yt*yt + xc;
        ytt = 2*xt*yt + yc;
        if (xt == xtt && yt == ytt)
          break;
        xt = xtt; yt = ytt;
        if (xt * xt * yt * yt > 16)
        {
            img[offst] = i % 256;
            return;
        }
    }
}
