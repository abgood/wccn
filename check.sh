#!/bin/bash

dirs=(/game/server/ /game/server1/)
log=log

for m in ${dirs[*]};
do
    if [ -f ${m}config.txt ]; then
        :>${log}
        master_site=`grep Site ${m}config.txt  | awk -F"\"" '{print $2}'`
        master_site_name=${master_site%_*}
        master_site_id=`echo ${master_site##*_} | sed -r 's/^0+([1-9])/\1/g'`
        array=($(grep "site='" ${m}/webapp/client/config.xml | awk -F"'" '{print $2}' | sort -n | xargs))
        for n in ${array[*]}
        do
            echo ${n} >> ${log}
        done
        
        master=""
        slave=""
        second=""
        
        all_site=(`awk -F"_" '{print $1}' ${log}| sort -n | uniq`)
        for i in ${all_site[*]}
        do
            id=(`grep ${i} ${log} | awk -F"_" '{print $2}' | sed -r 's/^0+([1-9])/\1/g' | sort -n | xargs`)
            output=""
            for j in `seq 1 ${#id[@]}`
            do
                begin=${id[$(($j-1))]}
                if [ 1 == $((${id[$j]}-$begin)) ];then
                    output="$output,$begin~${id[$j]}"
                else
                    output="$output,$begin"
                fi
            done
            output=`echo ${output/,/}|sed -r 's/([0-9]+),\1~//g;s/([^0-9])([0-9]+),\2/\1\2/g'`
            if [ "$i" == "$master_site_name" ]; then
                master="$i=$master_site_id-$output"
            else
                slave="$slave$i=$output;"
            fi
        done
        
        if [ -n "$slave" ]; then
            second=`echo $slave | sed 's/;$//' | sed 's/^/;/'`
        fi
	
        m_str=`echo $master | grep "~"`
        if [ -z "$m_str" ]; then
            master=${master%-*}
        fi

        # mysql里site字段生成
        site="$master$second"

        # mysql里ip字段生成
        telecom_ip=`ifconfig eth0 | grep "inet addr" | awk -F":" '{print $2}' | awk '{print $1}'`
        unicom_ip=`ifconfig eth0:1 | grep "inet addr" | awk -F":" '{print $2}' | awk '{print $1}'`

        # mysql里port字段生成
        port=`echo $m | grep -P -o "\d+"`
        if [ -z "$port" ]; then
            port=0
        fi

        # mysql里resoure字段生成
        line=`grep cdn ${m}/webapp/client/config.xml | wc -l`
        if [ line == 0 ]; then
            resoure=0
        else
            resoure=1
        fi

        # 插入mysql
        echo $site
        echo $telecom_ip
        echo $unicom_ip
        echo $port
        echo $resoure

    fi
done
