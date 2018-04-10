
With super user permissions run the following command to create the cluster
```shell
 kubeadm init --config kubeadm.yaml
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

Deploy the prometheus operator and wait until it is healthy:
```shell
 kubectl apply -f prometheus/prometheus-operator.yaml
 while [[ $(kubectl get prometheus; echo $?) == 1 ]]; do sleep 1; done
```
Deploy a prometheus instance and a custom metric server to connect our prometheus metrics to the kubernetes one. 
```shell
 kubectl apply -f prometheus/custom-metrics.yaml
 kubectl apply -f prometheus/prometheus-instance.yaml
```
Now everything is ready to deploy our services:
```shell
 kubectl apply -f picom-server.yaml
```

