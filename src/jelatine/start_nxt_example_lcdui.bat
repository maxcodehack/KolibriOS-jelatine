

set CLASSPATH_NXJ=..\classpath_nxj
set LEJOS_SAMPLES=..\..\..\leJOS_NXJ_0.9\leJosMyBuild\tests\lejos_org_samples

jelatine  -b %CLASSPATH_NXJ%  -s 4000000 --stack-size 640000 -c %LEJOS_SAMPLES%:%CLASSPATH_NXJ% org.lejos.sample.lcdui.LCDUI >rep 2>err
pause