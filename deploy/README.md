
Install docker, kubeadm, kubelet and kubectl version 1.9.x. Follow the tutorial [here](https://kubernetes.io/docs/setup/independent/install-kubeadm/) for that. 

Make sure swap is disabled:
```shell
 sudo swapoff -a
```

With super user permissions run the following command to create the cluster
```shell
 sudo kubeadm init --config kubeadm.yaml
```

To start using your cluster, you need to run the following as a regular user:
```shell
 mkdir -p $HOME/.kube
 sudo cp -i /etc/kubernetes/admin.conf $HOME/.kube/config
 sudo chown $(id -u):$(id -g) $HOME/.kube/config
```

You should now deploy a pod network to the cluster. Run the following command to deploy the weave plugin.
```shell
 kubectl apply -f "https://cloud.weave.works/k8s/net?k8s-version=$(kubectl version | base64 | tr -d '\n')"
```

By default, your cluster will not schedule pods on the master for security reasons. We can enable it by running:
```shell
 kubectl taint  nodes --all  node-role.kubernetes.io/master-
```

Deploy the message broker which connects our services:
```shell
 kubectl apply -f rabbitmq
```

Deploy infrastructure monitoring and visualization:
```shell
 kubectl apply -f heapster
```

Deploy the metrics server
```shell
 kubectl apply -f metrics-server
```

Deploy the prometheus operator and wait until it is ready:
```shell
 kubectl apply -f prometheus/prometheus-operator.yaml
 while [[ $(kubectl get prometheus; echo $?) == 1 ]]; do sleep 1; done
```

Deploy a prometheus instance and a custom metric server to connect our prometheus metrics to the kubernetes one. 
```shell
 kubectl apply -f prometheus/custom-metrics.yaml
 kubectl apply -f prometheus/prometheus-instance.yaml
```

Add the data sources to grafana by running:
```shell
 grafana=(`kubectl get svc --all-namespaces | grep -i monitoring-grafana`); grafana="${grafana[3]}"

 curl -H "Content-Type: application/json" -X POST -d '{ "name": "infra", "type": "influxdb", "url": "http://monitoring-influxdb.kube-system.svc:8086",  "access":"proxy", "database": "k8s" }' http://$grafana/api/datasources

 curl -H "Content-Type: application/json" -X POST -d '{ "name": "prometheus", "type": "prometheus", "url": "http://picom-prometheus.default.svc:9090",  "access":"proxy"}' http://$grafana/api/datasources
```

Now everything is ready to deploy our services. To deploy an experiment run:
```shell
 # where $N is the experiment id
 kubectl apply -f scenarios/$N
```

To visualize the experiment data just import the respective dashboard available on the dashboards directory.

To cleanup run:
```shell
 kubectl delete -f scenarios/$N
```

To cleanup everything:
```shell
 sudo kubeadm reset
```