#!/bin/bash
#测试udp服务器的可靠性,以及并发处理能力

testbatch=3000
i=0
make >/dev/null
>out
while [[ $i -lt $testbatch ]]
do
    ./udp_client 127.0.0.1 1 >>out &
    i=$((i+1))
done
sleep 10
wc -l out