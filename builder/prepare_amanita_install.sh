PROJECT_ROOT=/root/code

cd $PROJECT_ROOT
mkdir -p amanita_install amanita_install/config

cp $PROJECT_ROOT/_cmake_build/camerapro/camerapro $PROJECT_ROOT/amanita_install
cp $PROJECT_ROOT/_cmake_build/camerapro/config/camerapro.json $PROJECT_ROOT/amanita_install/config
cp $PROJECT_ROOT/_cmake_build/camerapro/config/camerapro-log.xml $PROJECT_ROOT/amanita_install/config

cp $PROJECT_ROOT/_cmake_build/datapro1/datapro1 $PROJECT_ROOT/amanita_install
cp $PROJECT_ROOT/_cmake_build/datapro1/config/config_datapro1.json $PROJECT_ROOT/amanita_install/config
cp $PROJECT_ROOT/_cmake_build/datapro1/config/dp1_log.xml $PROJECT_ROOT/amanita_install/config

cp $PROJECT_ROOT/_cmake_build/datapro2/datapro2 $PROJECT_ROOT/amanita_install
cp $PROJECT_ROOT/_cmake_build/datapro2/config/config_datapro2.json $PROJECT_ROOT/amanita_install/config
cp $PROJECT_ROOT/_cmake_build/datapro2/config/dp2_log.xml $PROJECT_ROOT/amanita_install/config

cp $PROJECT_ROOT/_cmake_build/turretpro/turretpro $PROJECT_ROOT/amanita_install
cp $PROJECT_ROOT/_cmake_build/turretpro/config/turretpro.json $PROJECT_ROOT/amanita_install/config
cp $PROJECT_ROOT/_cmake_build/turretpro/config/turretpro-log.xml $PROJECT_ROOT/amanita_install/config

cp $PROJECT_ROOT/_cmake_build/manager/manager $PROJECT_ROOT/amanita_install
cp $PROJECT_ROOT/_cmake_build/manager/config.json $PROJECT_ROOT/amanita_install/config