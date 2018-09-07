Руководство по примерам плагинов
===

## `ComposeAdd`

Этот эффект добавляет цвета в линейное цветовое пространство.

### input ports

| port name | |
| --- | --- |
| `layer[0-9]` | input images |

### parameters

None

## `ComposeMul`

Этот эффект умножает цвета в линейном цветовом пространстве.

### input ports

| port name | |
| --- | --- |
| `layer[0-9]` | input images. `layer0` is the back-most layer, and `layer9` is the front-most layer. `layer0` is required. |

### parameters

None

## `ComposeOptical`

Этот эффект составляет цвета, основанные на уравнении Кубельки-Мунка.

### input ports

| port name | |
| --- | --- |
| `layer[0-9]` | input images. `layer0` is the back-most layer, and `layer9` is the front-most layer. `layer0` is required. |

### parameters

| param name | default value | min value | max value | |
| --- | ---:| ---:| ---:| --- |
| `gamma`    | 2.2 | 0.100 | 5.0 | display gamma |
| `exposure` | 1.0 | 0.125 | 8.0 | экспозиция для расчета мощности света |

## `BlurChromaticAberration`

Этот эффект размывает входное изображение для каждого цветового канала.

### input ports

| port name | |
| --- | --- |
| `Input` | input image |

### parameters

| param name | default value | min value | max value | |
| --- | ---:| ---:| ---:| --- |
| `gamma`    |   2.20 | 0.100 |    5.0 | display gamma |
| `exposure` |   1.00 | 0.125 |    8.0 | exposure for calculating light power |
| `radius_r` |   0.01 | 1.000 |    1.0 | blur radius for red channel |
| `radius_g` |   0.01 | 1.000 |    1.0 | blur radius for green channel |
| `radius_b` |   0.01 | 1.000 |    1.0 | blur radius for blue channel |
| `margin`   | 100.00 | 0.000 | 1024.0 | margin width for blur |

## `BlurConvolution`

This effect blurs an input image by specified figures.

### input ports

| port name | |
| --- | --- |
| `Input` | input image |
| `A` | blur figure |
| `B` | blur figure |
| `C` | blur figure |

### parameters

| param name | default value | min value | max value | |
| --- | ---:| ---:| ---:| --- |
| `intensity_a` |   1 | 0 |    8 | intensity scaling factor for `A` |
| `intensity_b` |   1 | 0 |    8 | intensity scaling factor for `B` |
| `intensity_c` |   1 | 0 |    8 | intensity scaling factor for `C` |
| `scale_a`     |   1 | 0 |    8 | size scaling factor for `A` |
| `scale_b`     |   1 | 0 |    8 | size scaling factor for `B` |
| `scale_c`     |   1 | 0 |    8 | size scaling factor for `C` |
| `margin`      | 100 | 0 | 1024 | margin width for blur |

## `BlurMaskedC`

This is a circular blur effect which is specified blur intensity by a mask image.

### input ports

| port name | |
| --- | --- |
| `Input` | input image |
| `Mask` | mask image |

### parameters

| param name | default value | min value | max value | |
| --- | ---:| ---:| ---:| --- |
| `radius_r` | 1 | 0 | 16 | blur radius for red channel |
| `radius_g` | 1 | 0 | 16 | blur radius for green channel |
| `radius_b` | 1 | 0 | 16 | blur radius for blue channel |
| `radius_a` | 1 | 0 | 16 | blur radius for alpha channel |

## `BlurMaskedD`

This is a directional blur effect which is specified blur intensity by a mask image.

### input ports

| port name | |
| --- | --- |
| `Input` | input image |
| `Mask` | mask image |

### parameters

| param name | default value | min value | max value | |
| --- | ---:| ---:| ---:| --- |
| `radius_r` | 0.1 | 0 |   1 | blur radius for red channel |
| `radius_g` | 0.1 | 0 |   1 | blur radius for green channel |
| `radius_b` | 0.1 | 0 |   1 | blur radius for blue channel |
| `radius_a` | 0.1 | 0 |   1 | blur radius for alpha channel |
| `angle`    | 0.0 | 0 | 360 | blur angle in degree |

## `BlurMaskedR`

This is a radial blur effect which is specified blur intensity by a mask image.

### input ports

| port name | |
| --- | --- |
| `Input` | input image |
| `Mask` | mask image |

### parameters

| param name | default value | min value | max value | |
| --- | ---:| ---:| ---:| --- |
| `radius_r` | 0.1 | 0 | 1 | blur radius for red channel |
| `radius_g` | 0.1 | 0 | 1 | blur radius for green channel |
| `radius_b` | 0.1 | 0 | 1 | blur radius for blue channel |
| `radius_a` | 0.1 | 0 | 1 | blur radius for alpha channel |
| `x`        | 0.0 | 0 | 1 | x coordinate of center position |
| `y`        | 0.0 | 0 | 1 | y coordinate of center position |

## `BlurCurlNoise`

This is a LIC (line integral convolution) effect which is specified directions by [curl-noise (PDF)](https://www.cs.ubc.ca/~rbridson/docs/bridson-siggraph2007-curlnoise.pdf) generated form a mask image.

### input ports

| port name | |
| --- | --- |
| `Input` | input image |
| `Noise` | output buffer of `CoherentNoise` (required) |
| `Mask` | boundary condition of `curl-noise` |

### parameters

| param name | default value | min value | max value | |
| --- | ---:| ---:| ---:| --- |
| `gain`        | 16.0 | 0.1 | 16.0 | blur intensity |
| `attenuation` |  0.9 | 0.0 |  1.0 | attenuation rate for LIC |
| `debug`       |  0.0 | 0.0 |  1.0 | nois visualization flag for debugging |

## `LightBloom`

This effect generates light bloom by edge preservation blur.

### input ports

| port name | |
| --- | --- |
| `Input` | input image |

### parameters

| param name | default value | min value | max value | |
| --- | ---:| ---:| ---:| --- |
| `gamma`    |   2.2 | 0.100 |    5 | display gamma |
| `exposure` |   1.0 | 0.125 |    8 | exposure for calculating light power |
| `gain`     |   2.0 | 0.100 |   10 | bloom intensity |
| `radius`   |   5.0 | 1.000 |   32 | blur radius |
| `level`    |   8.0 | 0.000 |   10 | blur level |
| `margin`   | 100.0 | 0.000 | 1024 | margin width for blur |

## `LightGlare`

This effect generates light glare by discrete radial blur.

### input ports

| port name | |
| --- | --- |
| `Input` | input image |

### parameters

| param name | default value | min value | max value | |
| --- | ---:| ---:| ---:| --- |
| `gamma`       |   2.2 | 0.100 |    5.000 | display gamma |
| `exposure`    |   1.0 | 0.125 |    8.000 | exposure for calculating light power |
| `gain`        |   2.0 | 0.100 |   10.000 | glare intensity |
| `radius`      |   0.1 | 0.010 |    1.000 | glare radius |
| `attenuation` |   0.9 | 0.001 |    0.999 | glare attenuation |
| `number`      |   6.0 | 2.000 |   10.000 | the number of glare |
| `angle`       |  15.0 | 0.000 |  180.000 | glare angle in degree |
| `margin`      | 100.0 | 0.000 | 1024.000 | margin width for blur |

## `LightIncident`

This effect generates analog incident lights.

### input ports

| port name | |
| --- | --- |
| `Input` | input image |
| `Noise` | incident light intensity (required) |
| `Mask` | mask image |

### parameters

#### time

| param name | default value | min value | max value | |
| --- | ---:| ---:| ---:| --- |
| `time`       |  1.000 | 1 | 1500 | time parameter to determine internal state for calculating noise; a frame number is used as the time when `time` is `0` |
| `time_limit` |  8.000 | 2 |  250 | loop point of the `time`. `time` is looped in `[1,time_limit]` |
| `beta`       | 10.000 | 0 |   30 | time-dependent range of incident light direction |
| `gamma`      |  0.001 | 0 |    1 | time-dependent sharpness of incident light direction |

#### geometry

| param name | default value | min value | max value | |
| --- | ---:| ---:| ---:| --- |
| `distance`     |  2.00 |    0.00 |   5 | distance from a center of the frame |
| `theta`        | 40.00 | -180.00 | 180 | light angle |
| `phi`          | 30.00 |    0.00 |  90 | light direction range |
| `alpha`        |  0.00 |  -45.00 |  45 | light related angle |
| `width`        |  0.10 |    0.00 |  30 | light width |
| `length`       |  2.00 |    0.01 |  10 | light length |
| `scraggly`     |  0.20 |    0.01 |   2 | light length variability |
| `roughness`    |  0.02 |    0.01 |   1 | oval length |
| `distinctness` |  0.50 |    0.01 |   2 | oval width |
| `number`       | 40.00 |    1.00 | 100 | the number of incident lights |

#### color

| param name | default value | min value | max value | |
| --- | ---:| ---:| ---:| --- |
| `r`         | 1 | 0.00 |   1 | incident light color (red) |
| `g`         | 1 | 0.00 |   1 | incident light color (green) |
| `b`         | 1 | 0.00 |   1 | incident light color (blue) |
| `intensity` | 2 | 0.01 | 100 | incident light intensity |

#### figure

| param name | default value | min value | max value | |
| --- | ---:| ---:| ---:| --- |
| `blur` | 0.01 | 0 | 0.5 | softness of light figure |

#### falloff

| param name | default value | min value | max value | |
| --- | ---:| ---:| ---:| --- |
| `falloff`     | 0.8 | 0.00 | 1.0 | attenuation distance |
| `sensitivity` | 0.1 | 0.01 | 2.0 | attenuation shape |

#### dispersion

| param name | default value | min value | max value | |
| --- | ---:| ---:| ---:| --- |
| `d_rate` | 7.0 | 0 | 8 | color rate |
| `d_bias` | 0.5 | 0 | 8 | color bias |
| `d_gain` | 0.3 | 0 | 1 | color intensity |

#### bloom

| param name | default value | min value | max value | |
| --- | ---:| ---:| ---:| --- |
| level | 6.0 | 0 | 32 | bloom intensity |
| gain  | 1.0 | 0 |  8 | brightness intensity |
| bias  | 0.0 | 0 |  8 | brightness bias |

#### system

| param name | default value | min value | max value | |
| --- | ---:| ---:| ---:| --- |
| `seed_intensity` | 0.99 | 0 | 1 | random seed of random number generator for incident light intensity |
| `seed_direction` | 0.98 | 0 | 1 | random seed of random number generator for incident light direction |
| `seed_width`     | 0.97 | 0 | 1 | random seed of random number generator for incident light width |
| `seed_length`    | 0.96 | 0 | 1 | random seed of random number generator for incident light length |
| `seed_gamma`     | 0.95 | 0 | 1 | random seed of random number generator for incident light time |
| `seed_phase`     | 0.94 | 0 | 1 | random seed of random number generator for incident light position |

## `CoherentNoise`

This effect generates [Perlin Noise (PDF)](http://mrl.nyu.edu/~perlin/paper445.pdf) for `Noise` of `BlurCurlNoise`, `LightIncident`, and `WaveGlass` effects.

### input ports

| port name | |
| --- | --- |
| `Input` | same `Input` as `BlurCurlNoise`, `LightIncident` and `WaveGlass` effects |

### parameters

| param name | default value | min value | max value | |
| --- | ---:| ---:| ---:| --- |
| `time`       | 1.00 | 1 | 1500 | time parameter to determine internal state for calculating noise; a frame number is used as the time when `time` is `0` |
| `time_limit` | 8.00 | 2 |  250 | loop point of the `time`. `time` is looped in `[1,time_limit]` |
| `alpha`      | 0.80 | 0 |    1 | smoothness of time coherence |
| `gain`       | 1.00 | 0 |    1 | noise gain |
| `bias`       | 0.50 | 0 |    1 | noise bias |
| `amp0`       | 1.00 | 0 |    1 | low-frequency intensity |
| `amp1`       | 0.80 | 0 |    1 | double `amp0` frequency intensity |
| `amp2`       | 0.60 | 0 |    1 | double `amp1` frequency intensity |
| `amp3`       | 0.40 | 0 |    1 | double `amp2` frequency intensity |
| `amp4`       | 0.20 | 0 |    1 | double `amp3` frequency intensity |
| `seed`       | 0.50 | 0 |    1 | random seed of random number generator |

## `Drip`

This effect extracts bright portions by a threshold.

### input ports

| port name | |
| --- | --- |
| `Input` | input image |

### parameters

| param name | default value | min value | max value | |
| --- | ---:| ---:| ---:| --- |
| `threshold` | 0.9 | 0 | 1 | threshold of brightness |

## `Paraffin`

This effect places a paraffin.

### input ports

| port name | |
| --- | --- |
| `Input` | input image |

### parameters

| param name | default value | min value | max value | |
| --- | ---:| ---:| ---:| --- |
| `distance` | -1.0 |   -1.5 |   1.5 | related distance from a center of the frame |
| `theta`    | 40.0 | -180.0 | 180.0 | paraffin angle |
| `radius`   |  0.1 |    0.0 |   1.0 | blur radius |
| `red`      |  0.0 |    0.0 |   1.0 | paraffin color (red) |
| `green`    |  0.0 |    0.0 |   1.0 | paraffin color (green) |
| `blue`     |  0.0 |    0.0 |   1.0 | paraffin color (blue) |

## `PencilHatching`

This effect applied LIC (line integral convolution) to an input image.

### input ports

| port name | |
| --- | --- |
| `Input` | input image |

### parameters

| param name | default value | min value | max value | |
| --- | ---:| ---:| ---:| --- |
| `angle`       | 0.00 | 0 | 360 | hatching angle in degree |
| `length`      | 0.01 | 0 |   1 | hatching length |
| `attenuation` | 0.90 | 0 |   1 | attenuation rate for LIC |
| `seed`        | 0.25 | 0 |   1 | random seed for random number generator |

## `WaveGlass`

This effect simulates light propagation through a glass.

### input ports

| port name | |
| --- | --- |
| `Input` | input image |
| `Noise` | displacement of a glass which is an output buffer of `CoherentNoise` (required) |
| `Mask` | displacement intensity |

### parameters

| param name | default value | min value | max value | |
| --- | ---:| ---:| ---:| --- |
| `gain`   | 1 | 0 | 32.00 | displacement intensity |
| `eta`    | 2 | 1 |  2.50 | index of refraction |
| `height` | 1 | 0 | 32.00 | distance between a level and a glass |
| `depth`  | 1 | 0 |  2.00 | glass thickness |
| `red`    | 0 | 0 |  0.50 | absorption coefficient (red) of glass |
| `green`  | 0 | 0 |  0.50 | absorption coefficient (green) of glass |
| `blue`   | 0 | 0 |  0.50 | absorption coefficient (blue) of glass |
| `blur`   | 8 | 0 |  0.01 | intensity of low-pass filter |

## `Kaleidoscope`

This effect generates a kaleidoscope view.

### input ports

| port name | |
| --- | --- |
| `Input` | input image |

### parameters

| param name | default value | min value | max value | |
| --- | ---:| ---:| ---:| --- |
| `number` |  3.0 | 3.00 |  10.00 | the number of mirrors |
| `angle`  |  0.0 | 0.00 | 360.00 | mirror angle in degree |
| `x`      |  0.5 | 0.00 |   1.00 | x coordinate of center position |
| `y`      |  0.5 | 0.00 |   1.00 | y coordinate of center position |
| `radius` |  0.5 | 0.00 |   1.00 | radius of kaleidoscope |
| `albedo` |  0.7 | 0.01 |   0.99 | reflectance rate of mirrors |
| `depth`  | 10.0 | 0.00 | 100.00 | max number of reflections |

## `Tiling`

This is a tiling effect.

### input ports

| port name | |
| --- | --- |
| `Input` | input image |

### parameters

| param name | default value | min value | max value | |
| --- | ---:| ---:| ---:| --- |
| `mirroring` | 0 | 0 | 1 | flag of mirroring |

## `ImageQuilting`

This effect applies border optimization of [image quilting (PDF)](http://www.eecs.berkeley.edu/Research/Projects/CS/vision/papers/efros-siggraph01.pdf) to two input images.

### input ports

| port name | |
| --- | --- |
| `background` | background image |
| `foreground` | foreground image |

### parameters

| param name | default value | min value | max value | |
| --- | ---:| ---:| ---:| --- |
| `border` | 0 | 0.2 | 0 | 1 | border width |
| `debug`  | 0 | 0.0 | 0 | 1 | show borders for debugging |
