import lbann_proto as lp

blocks = [3, 4, 6, 3]  # Blocks for ResNet-50.
bn_stats_aggregation = 'global'

class ConvBNRelu2d:
    """Convolution -> Batch normalization -> ReLU"""

    def __init__(self, name, out_channels, kernel_size,
                 stride=1, padding=0, dilation=1, bias=False, relu=True,
                 bn_stats_aggregation='global'):
        self.step = 0
        self.name = name
        self.out_channels = out_channels
        self.kernel_size = kernel_size
        self.stride = stride
        self.padding = padding
        self.dilation = dilation
        self.bias = bias
        self.relu = relu
        self.bn_stats_aggregation = bn_stats_aggregation

    def __call__(self, x):
        name = ('{0}_step{1}'.format(self.name, self.step)
                if self.step
                else self.name)
        self.step += 1
        conv = lp.Convolution(x,
                              name=name+'_conv',
                              num_dims=2,
                              num_output_channels=self.out_channels,
                              has_vectors=False,
                              conv_dims_i=self.kernel_size,
                              conv_pads_i=self.padding,
                              conv_strides_i=self.stride,
                              conv_dilations_i=self.dilation,
                              has_bias=self.bias)
        bn = lp.BatchNormalization(conv,
                                   name=name+'_bn',
                                   decay=0.9, epsilon=1e-5,
                                   stats_aggregation=bn_stats_aggregation)
        if self.relu:
            return lp.Relu(bn, name=name+'_relu')
        else:
            return bn

class ResBottleneck:
    """ResNet bottleneck building block."""

    def __init__(self, name, mid_channels, out_channels, stride,
                 dilation=1, downsample=False,
                 bn_stats_aggregation='global'):
        self.step = 0
        self.name = name
        self.conv1 = ConvBNRelu2d(name + '_conv1', mid_channels, 1,
                                  stride=1, padding=0, dilation=1,
                                  bn_stats_aggregation=bn_stats_aggregation)
        self.conv2 = ConvBNRelu2d(name + '_conv2', mid_channels, 3,
                                  stride=stride, padding=dilation, dilation=dilation,
                                  bn_stats_aggregation=bn_stats_aggregation)
        self.conv3 = ConvBNRelu2d(name + '_conv3', out_channels, 1,
                                  stride=1, padding=0, dilation=1, relu=False,
                                  bn_stats_aggregation=bn_stats_aggregation)
        if downsample:
            self.downsample = ConvBNRelu2d(name + '_proj', out_channels, 1,
                                           stride=stride, padding=0,
                                           dilation=1, relu=False,
                                           bn_stats_aggregation=bn_stats_aggregation)
        else:
            self.downsample = None

    def __call__(self, x):
        name = ('{0}_step{1}'.format(self.name, self.step)
                if self.step
                else self.name)
        self.step += 1
        conv = self.conv3(self.conv2(self.conv1(x)))
        if self.downsample is None:
            residual = x
        else:
            residual = self.downsample(x)
        sum = lp.Sum([conv, residual], name=name+'_sum')
        return lp.Relu(sum, name=name+'_relu')

class ResBlock:
    """ResNet block, constructed of some number of bottleneck layers."""

    def __init__(self, name, num_layers, mid_channels, out_channels,
                 stride, dilation=1, bn_stats_aggregation='global'):
        self.layers = []
        self.layers.append(ResBottleneck(
            name + '_bottleneck1', mid_channels, out_channels,
            stride, dilation=dilation, downsample=True,
            bn_stats_aggregation=bn_stats_aggregation))
        for i in range(num_layers - 1):
            self.layers.append(ResBottleneck(
                name + '_bottleneck{0}'.format(i+2), mid_channels,
                out_channels, stride=1, dilation=dilation, downsample=False,
                bn_stats_aggregation=bn_stats_aggregation))

    def __call__(self, x):
        for l in self.layers:
            x = l(x)
        return x

# Construct layer graph.
input = lp.Input(io_buffer='partitioned')
images = lp.Identity(input)
labels = lp.Identity(input)
conv1 = ConvBNRelu2d('conv1', 64, 7, stride=2, padding=3,
                     bn_stats_aggregation=bn_stats_aggregation)(images)
pool1 = lp.Pooling(conv1, num_dims=2, has_vectors=False,
                   pool_dims_i=3, pool_pads_i=1, pool_strides_i=2,
                   pool_mode='max')
block1 = ResBlock('block1', blocks[0], 64, 256, stride=1,
                  bn_stats_aggregation=bn_stats_aggregation)(pool1)
block2 = ResBlock('block2', blocks[1], 128, 512, stride=2,
                  bn_stats_aggregation=bn_stats_aggregation)(block1)
block3 = ResBlock('block3', blocks[2], 256, 1024, stride=2,
                  bn_stats_aggregation=bn_stats_aggregation)(block2)
block4 = ResBlock('block4', blocks[3], 512, 2048, stride=2,
                  bn_stats_aggregation=bn_stats_aggregation)(block3)
avgpool = lp.Pooling(block4, num_dims=2, has_vectors=False,
                     pool_dims_i=7, pool_pads_i=0, pool_strides_i=1,
                     pool_mode='average')
fc = lp.FullyConnected(avgpool, hint_layer=labels, has_bias=False)
softmax = lp.Softmax(fc)
ce = lp.CrossEntropy([softmax, labels])
top1 = lp.CategoricalAccuracy([softmax, labels])
top5 = lp.TopKCategoricalAccuracy([softmax, labels], k=5)
layers = list(lp.traverse_layer_graph(input))

# Explicitly set up weights for all layers.
l2_reg_weights = []
for l in layers:
    if type(l) == lp.Convolution:
        l.add_weights(lp.Weights(l.name + '_kernel', lp.HeNormalInitializer()))
        l2_reg_weights.extend(l.weights)
    elif type(l) == lp.BatchNormalization:
        init_scale = 0.0 if ('conv3' in l.name) else 1.0
        l.add_weights([lp.Weights(l.name + '_scale',
                                  lp.ConstantInitializer(value=init_scale)),
                       lp.Weights(l.name + '_bias',
                          lp.ConstantInitializer(value=0.0))])
    elif type(l) == lp.FullyConnected:
        l.add_weights(lp.Weights(
            l.name + '_matrix',
            lp.NormalInitializer(mean=0.0, standard_deviation=0.01)))
        l2_reg_weights.extend(l.weights)

# Set up other model components.
obj = lp.ObjectiveFunction(
          [ce, lp.L2WeightRegularization(weights=l2_reg_weights, scale=1e-4)])
metrics = [lp.Metric(top1, name='categorical accuracy', unit='%'),
           lp.Metric(top5, name='top-5 categorical accuracy', unit='%')]
callbacks = [lp.CallbackPrint(),
             lp.CallbackTimer(),
             lp.CallbackDropFixedLearningRate(
                 drop_epoch=[30, 60, 80], amt=0.1)]

# Export model to file
lp.save_model('resnet50.prototext', 256, 90,
              layers=layers, objective_function=obj,
              metrics=metrics, callbacks=callbacks)
