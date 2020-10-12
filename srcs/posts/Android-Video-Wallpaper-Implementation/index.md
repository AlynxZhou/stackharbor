---
title: 如何编写 Android 视频壁纸
layout: post
#comment: true
created: 2019-03-22 16:18:00
categories:
  - 计算机
  - 编程
  - Android
tags:
  - Android
  - Java
  - OpenGL
---
最近我编写了一个 Android 的视频壁纸应用（[GitHub Repo](https://github.com/AlynxZhou/AlynxLiveWallpaper)），一开始觉得并没有什么难写的地方，应该很快就可以写出来，但是后来发现我想的太简单了。或许你也看过许多写视频壁纸的教程，但我发现他们都有一些问题，写出来的程序基本不能用，所以我打算在这里写一下如何编写一个可以发布的 Android 视频壁纸，而不是一个 demo。

<!--more-->

首先你需要新建一个项目，通常应该都是使用 Android Studio，我就忽略你如何与网络搏斗让 Android Studio 可用了。什么程序模板可以随便选，反正你还是要自己新建 class。最低 API 版本不要太低就行，如果你只是写一个动态壁纸，应该不需要很高的 API，因为 Android 很早就支持动态壁纸了（但为了让开发方便一点，建议只考虑 5.0 以上的就行了，反正现在即使有 5.0 以下的机器多半也成垃圾了）。

# 首先让系统知道你是个壁纸……

在开始写逻辑代码之前，我们还得先让系统知道你写的这是个壁纸，这需要一些起声明作用的代码。先把你 Android Studio 左侧侧栏调成文件树模式（忘了什么诡异的分类模式吧，不然你连文件在哪都找不到，试图隐藏细节只会让事情变的更复杂）。

应该有一个自动生成的 `AndroidManifest.xml` 文件，我觉得你应该比我更了解这玩意是干嘛的，因为实际上我没系统地研究过 Android，我只是会一点点 Java 而已——然后靠不停的查看手册来自学。加上这两行：

```xml
<uses-permission android:name="android.permission.SET_WALLPAPER" />
<uses-feature
  android:name="android.software.live_wallpaper"
  android:required="true" />
```

你要是能把这一行加到 `<manifest>` 标签外面，我觉得你也没必要写代码了……但这个要放在 `<application>` 标签的外面，具体放在哪直接翻我仓库里的文件好了，比我用文字描述清晰的多。

但是下面这个要加到 `<application>` 标签的里面：

```xml
<service
  android:name=".GLWallpaperService"
  android:permission="android.permission.BIND_WALLPAPER">
  <intent-filter>
    <action android:name="android.service.wallpaper.WallpaperService" />
  </intent-filter>
  <meta-data
    android:name="android.service.wallpaper"
    android:resource="@xml/wallpaper" />
</service>
```

看到那个叫 `android:name` 的属性了吗，`.` 不能少，后面的名字则是一会你的 class 叫什么名字这里就什么名字。然后别的都是套路，可以不管，除了那个 `@xml/wallpaper`，这玩意要自己写。

在你侧栏找到 `res` 目录，如果底下没有叫 `xml` 的目录的话，你就新建一个。然后你刚才那里斜杠后面是什么你就建立一个叫什么的 xml 文件，比如我写了 `@xml/wallpaper`，那就建立一个 `wallpaper.xml` 的文件，内容如下：

```xml
<?xml version="1.0" encoding="UTF-8"?>
<wallpaper
  xmlns:android="http://schemas.android.com/apk/res/android"
  android:thumbnail="@drawable/ic_icon"
  android:description="@string/wallpaper_description"
  android:settingsActivity="xyz.alynx.livewallpaper.MainActivity" />
```

这里面也有些地方需要改，首先那个 `@drawable/ic_icon` 是你这个程序在系统的动态壁纸列表里面的图标（然而大部分定制的系统都忽略了这玩意），你可以用 Android Studio 在 `res/drawable` 目录下面生成一个，这个可以自己去查官方文档，`ic_icon` 要换成你生成的文件的名字。`@string/wallpaper_description` 则是你的程序在列表里的名字或介绍，这个格式的意思是去 `res/values/strings.xml` 或者其他语言的 `strings.xml` 里面找字符串（写过点复杂程序的人应该都懂吧！不懂自学去！我这不是教你怎么写代码！），你可以用 Android Studio 的字符串编辑器添加一条，然后把这里的键改成你添加的。最后一个 `settingsActivity` 是干嘛的呢？在预览动态壁纸的页面会有个设置按钮，如果你的动态壁纸可以设置参数，点击那个按钮会跳进动态壁纸设置页面，设置页面就是在这里指定的类，因为我设置项不多直接放菜单里了，所以我写的是直接跳进 `MainActivity`，你写的时候也可以这样，但记得换成你的包名（不换的人我怀疑你没写过 Java，那你学个毛 Android App 开发？）。

废话终于说完了可以写代码了好开心啊……

# 然后你看见网上一堆胶水代码……

如果你找过网上的教程，你会觉得视频壁纸是很简单的，他们都告诉你播放视频有现成的组件，你只要写几行胶水代码把它和壁纸连起来就行了……

行个头！要是那么简单我就不用写这一篇文章了好吗？

我们先从最基础的知识开始，动态壁纸实际上是一个 `Service`，你去实现一个 `Service`，然后系统来调用，你需要继承一个叫做 `WallpaperService` 的类，实现里面的一些方法。但实际上这个类只是个包装，真正的工作代码是里面一个叫 `Engine` 的类，具体的动态壁纸实现都是在它里面。

所以你需要建立一个随便你爱叫什么的 java 文件（我不会用 Kotlin，但你愿意的话也可以用）来存放你自己的 `Service`……或者用 Android Studio 的向导也不错。比如我的就叫 `GLWallpaperService`，里面的代码看起来大概像这样：

```java
package xyz.alynx.livewallpaper;

import android.service.wallpaper.WallpaperService;

public class GLWallpaperService extends WallpaperService {
    public class GLWallpaperEngine extends Engine {

    }

    @Override
    public Engine onCreateEngine() {
        return new GLWallpaperEngine();
    }
}
```

所以你看，你只需要实现 `WallpaperService` 的一个方法，就是在系统需要 `Engine` 的时候，返回你自己的 `Engine` 实例。

然后你需要在你的 `Engine`（我的是 `GLWallpaperEngine`）里面实现一些系统要求的方法，我们先说最基础的几个：

```java
        @Override
        public void onSurfaceCreated(SurfaceHolder surfaceHolder) {
            super.onSurfaceCreated(surfaceHolder);
        }

        @Override
        public void onVisibilityChanged(boolean visible) {
            super.onVisibilityChanged(visible);
        }

        @Override
        public void onSurfaceDestroyed(SurfaceHolder holder) {
           super.onSurfaceDestroyed(holder);
        }
        @Override
        public void onOffsetsChanged(
            float xOffset, float yOffset,
            float xOffsetStep, float yOffsetStep,
            int xPixelOffset, int yPixelOffset
        ) {
            super.onOffsetsChanged(
                xOffset, yOffset, xOffsetStep,
                yOffsetStep, xPixelOffset, yPixelOffset
            );
        }
```

Android 的动态壁纸实际上是一个 `Surface`（Android 的 `Surface` 你可以理解为画布……大概吧），系统调用你的 `Engine`，然后给你一个可以绘制的 `SurfaceHolder`，你只要往这个 `SurfaceHolder` 里面的 `Surface` 画东西就行了。`onSurfaceCreated()` 表示画布准备好了，`onVisibilityChanged()` 则表示画布看不见了或者又出来了，`onSurfaceDestroyed()` 表示画布没了（我不是你英语老师！），`onOffsetsChanged()` 则是桌面通知壁纸偏移量，通常是实现壁纸随桌面滚动的效果。

不要把他们和 `onCreate()` 和 `onDestoryed()` 搞混了，那俩说的是 `Engine`，但我们做绘制工作时候操心的是画布。

为什么说是个胶水代码呢……因为 Android 内建了一个播放器叫 `MediaPlayer`，它有个方法叫 `setSurface()`，可以把视频渲染到画布上，然后我们就美滋滋地看……到 bug。

怎么设置 `MediaPlayer` 我就不讲了，因为网上有很多现成的，总之就是实例化设置静音设置来源再 `setSurface(getSurfaceHolder().getSurface())`（`getSurfaceHolder()` 是 `Engine` 的，可以获取当前的 `SurfaceHolder`），然后开始播放，你需要在 `onSurfaceCreated()` 的时候弄出个播放器，`onSurfaceDestroyed()` 的时候 `release()` 掉播放器，然后在由可见变成不可见的时候保存进度删掉播放器（其实暂停也行，但是暂停仍然占据着资源），在可见时候建立播放器并回复到之前的进度（如果看不见了还一直播，你手机的电池估计坚持不了几小时）。反正代码网上一大堆，我就不贴了。

然后你就可以运行这个程序啦，如果你用的模拟器，你就美滋滋的发现，你的视频被硬生生压缩或拉伸成了桌面比例，视频比例没了。大部分视频应该都是横向更长一点，而手机竖向长，所以……

如果你开始翻文档，你会发现一个 `setVideoScalingMode()` 的方法，可以设置为 `FIT_WITH_CROPPING`，你想着这不就是我们要的效果嘛：把视频放屏幕中间，然后裁出正中间最大的一块……当然不行啦！不然我写什么！

很高兴告诉你这个其实是包装了系统的 `MediaCodec` 的参数，但这个实现及其不靠谱，比如在模拟器上这个功能根本就没实现！在我的 Galaxy S9+ 上虽然实现了，但在切换界面的时候（比如锁屏啦，打开应用界面啦），会突然变成没有剪裁的，总之你会看到一个奇怪的切换（这哪个程序员写的出来祭天）。

所以如果你想正确剪裁视频的话……当然要自己动手！Android 的 `View` 是可以设置尺寸缩放没错，但是别忘了，这里只有 `Surface`，没有 `View`，所以没戏。用 CPU 剪裁视频的话（剪裁不是剪辑！）怕是你一帧剪完那边 30 帧都播完了，解决方案就是用 OpenGL ES 来进行这个操作。

# 明明是杀鸡，还是得掏牛刀！

如果你比较熟悉 OpenGL 的话，其实这个剪裁本身的难度比构建一个 OpenGL 环境的难度低的多……只要算一下比例做个缩放矩阵就行了，视频是矩形所以我们就先弄个正方形出来然后把每一帧解码的画面当成纹理贴上去。

解码的工作我们可以交给 `MediaPlayer`，然后因为性能需要，我们不能每一帧都保存为图片然后再转读为 OpenGL 的纹理，Android 有一个 OpenGL 的扩展，可以把视频解码到的 `Surface` 作为一个纹理，大部分机型都支持，不支持的机型其实也可以告别动态壁纸了，啥都不支持，你还不给我钱，我为啥要给你写代码？

但是这个过程需要一些 hack。

显然系统不给你现成的 `GLSurfaceView`，而只是一个 `SurfaceHolder`，所以正常的思路是自己弄 `GLThread` 白手起家……并不，我们在 `Engine` 里插入下面的代码：

```java
        private class GLWallpaperSurfaceView extends GLSurfaceView {
            private static final String TAG = "GLWallpaperSurface";

            public GLWallpaperSurfaceView(Context context) {
                super(context);
            }

            @Override
            public SurfaceHolder getHolder() {
                return getSurfaceHolder();
            }

            public void onDestroy() {
                super.onDetachedFromWindow();
            }
        }
```

看到那个 `getHolder()` 了吗？没错！`GLSurfaceView` 似乎内部自己有个 `SurfaceHolder`，我们 hack 一下它，扩展出一个 `GLWallpaperSurfaceView`，因为它是个内部类，当它要往自己的 `SurfaceHolder` 上渲染时，我们偷梁换柱，返回它 `Engine.getSurfaceHolder()`，可怜的家伙还蒙在鼓里，自己内部的 `SurfaceHolder` 已经没用了，现在它画的实际上是桌面壁纸的 `Surface`。

我也不知道谁先想出的这个套路，但第一个想出这个办法的老哥可真是个天才。

至于你问 `Context` 从哪来？`Service` 就是个 `Context`，所以你改写一下 `Engine` 的构造函数把 `Service` 传进去，然后创建 `GLWallpaperSurfaceView` 时传给它就行了，基本操作基本操作。

然后我们就可以实现 `GLSurfaceView.Renderer` 了，这是个已经包装好的 OpenGL Renderer，我们只要负责在里面画就行了。至于 GLES 的版本，如果你想省事，只用 v2 就好了，除非像我一样对 v3 很熟悉，当然我 v2 v3 都写了 Renderer，能支持 v3 的先调用 v3（除了模拟器不支持，手机基本都支持了）。我这里用 v3 做例子，你应该自己也会写 v2 吧？如果你只会写固定管线，请你不要说自己会 OpenGL。

当然最好在 `AndroidManifest.xml` 声明一下你这个应用需要的 GLES 版本：

```xml
<uses-feature
  android:glEsVersion="0x00020000"
  android:required="true" />
```

如果你只支持 v3，就把 2 改成 3。

（已经进入这个阶段了，打算拿这个当“我的第一个 Android App”的同学就自己退出吧。）

我直接帖完整代码解释好了，因为实在是不好一点点讲了，反正会写 OpenGL 的人应该也能理解这种程度的代码了。

我先把 Shader 代码贴上来，我是把它们放在 `res/raw` 下面的，Android 会自动生成 `R.raw.` 的 ID。

- vertex shader

```glsl
#version 300 es

// Some Android phone driver needs to put pre-processor in the first line. (e.g. Huawei Kirin)

layout(location = 0) in vec2 in_position;
layout(location = 1) in vec2 in_tex_coord;

uniform mat4 mvp;

out vec2 tex_coord;

void main() {
    gl_Position = mvp * vec4(in_position, 1.0f, 1.0f);
    tex_coord = in_tex_coord;
}
```

应该在你画三角形的时候就学会这玩意了吧？注意有些手机的实现要求第一行不能是注释，否则会报错。

- fragment shader

```glsl
#version 300 es
#ifdef GL_OES_EGL_image_external_essl3
#extension GL_OES_EGL_image_external_essl3 : require
#else
#extension GL_OES_EGL_image_external : require
#endif

// Some Android phone driver needs to put pre-processor in the first line. (e.g. Huawei Kirin)

precision mediump float;

uniform samplerExternalOES frame;

in vec2 tex_coord;
out vec4 frag_color;

void main() {
    frag_color = texture(frame, tex_coord);
}
```

注意开头的判断！本来在 GLESv3 里面，对这种 Android 直接拿 `Surface` 当纹理的 flag 是 `GL_OES_EGL_image_external_essl3`，`GL_OES_EGL_image_external` 已经被废弃了，但有些支持 GLESv3 的手机只支持旧的表达方法，所以我们判断一下就好。GLESv2 则直接用后面那个，不要用前面的。

同样，纹理类型也要换成 `samplerExternalOES`，别的都没关系了。

编译和链接的过程我直接写成工具函数了，连同加载 `res/raw` 的代码，我贴上来：

```java
static int compileShaderResourceGLES30(
    @NonNull Context context,
    final int shaderType,
    final int shaderRes
) throws RuntimeException {
    final InputStream inputStream = context.getResources().openRawResource(shaderRes);
    final BufferedReader bufferedReader = new BufferedReader(new InputStreamReader(inputStream));
    String line;
    final StringBuilder stringBuilder = new StringBuilder();
    try {
        while ((line = bufferedReader.readLine()) != null) {
            stringBuilder.append(line);
            stringBuilder.append('\n');
        }
    } catch (IOException e) {
        e.printStackTrace();
        return 0;
    }
    final String shaderSource = stringBuilder.toString();
    int shader = GLES30.glCreateShader(shaderType);
    if (shader == 0) {
        throw new RuntimeException("Failed to create shader");
    }
    GLES30.glShaderSource(shader, shaderSource);
    GLES30.glCompileShader(shader);
    final int[] status = new int[1];
    GLES30.glGetShaderiv(shader, GLES30.GL_COMPILE_STATUS, status, 0);
    if (status[0] == 0) {
        final String log = GLES30.glGetShaderInfoLog(shader);
        GLES30.glDeleteShader(shader);
        throw new RuntimeException(log);
    }
    return shader;
}

static int linkProgramGLES30(
    final int vertShader,
    final int fragShader
) throws RuntimeException {
    int program = GLES30.glCreateProgram();
    if (program == 0) {
        throw new RuntimeException("Failed to create program");
    }
    GLES30.glAttachShader(program, vertShader);
    GLES30.glAttachShader(program, fragShader);
    GLES30.glLinkProgram(program);
    final int[] status = new int[1];
    GLES30.glGetProgramiv(program, GLES30.GL_LINK_STATUS, status, 0);
    if (status[0] == 0) {
        final String log = GLES30.glGetProgramInfoLog(program);
        GLES30.glDeleteProgram(program);
        throw new RuntimeException(log);
    }
    return program;
}
```

当然还是那个令人头疼的问题，你需要传个 `Context`，这个问题不大，`Engine` 创建 `Renderer` 的时候传给它就好了。

`Renderer` 则是按下面的写：

```java
package xyz.alynx.livewallpaper;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.support.annotation.NonNull;

public class GLWallpaperRenderer implements GLSurfaceView.Renderer {
    private static final int BYTES_PER_FLOAT = 4;
    private static final int BYTES_PER_INT = 4;
    private Context context;
    private FloatBuffer vertices;
    private FloatBuffer texCoords;
    private IntBuffer indices;
    private float[] mvp;
    private int program = 0;
    private int mvpLocation = 0;
    private int[] textures;
    private SurfaceTexture surfaceTexture = null;
    private int screenWidth = 0;
    private int screenHeight = 0;
    private int videoWidth = 0;
    private int videoHeight = 0;
    private int videoRotation = 0;
    private float xOffset = 0;
    private float yOffset = 0;
    private long updatedFrame = 0;
    private long renderedFrame = 0;
```

上面这部份的变量都是一会要用到的，一会再说。

```
    GLWallpaperRenderer(@NonNull final Context context) {
        this.context = context;

        // Those replaced glGenBuffers() and glBufferData().
        final float[] vertexArray = {
            // x, y
            // bottom left
            -1.0f, -1.0f,
            // top left
            -1.0f, 1.0f,
            // bottom right
            1.0f, -1.0f,
            // top right
            1.0f, 1.0f
        };
        vertices = ByteBuffer.allocateDirect(
            vertexArray.length * BYTES_PER_FLOAT
        ).order(ByteOrder.nativeOrder()).asFloatBuffer();
        vertices.put(vertexArray).position(0);

        final float[] texCoordArray = {
            // u, v
            // bottom left
            0.0f, 1.0f,
            // top left
            0.0f, 0.0f,
            // bottom right
            1.0f, 1.0f,
            // top right
            1.0f, 0.0f
        };
        texCoords = ByteBuffer.allocateDirect(
            texCoordArray.length * BYTES_PER_FLOAT
        ).order(ByteOrder.nativeOrder()).asFloatBuffer();
        texCoords.put(texCoordArray).position(0);

        final int[] indexArray = {
            0, 1, 2,
            3, 2, 1
        };
        indices = ByteBuffer.allocateDirect(
            indexArray.length * BYTES_PER_INT
        ).order(ByteOrder.nativeOrder()).asIntBuffer();
        indices.put(indexArray).position(0);

        textures = new int[1];

        mvp = new float[] {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
    }

    @NonNull
    public Context getContext() {
        return context;
    }
```

这部份很好懂吧？先处理 `Context`，然后我们准备 OpenGL 需要的坐标，正方形的模型坐标，纹理坐标，以及一个索引 buffer，不同的是在 Android 里面不用写 `glGenBuffers`、`glBindBuffer` 了，我们用 java 的 nio 准备数组，然后 Android 可以直接加载这里面的 buffer。（但是 mvp 矩阵仍然是数组。）

当然，这里是没有 OpenGL 的上下文的，所以我们现在不能处理 OpenGL 的设置，需要实现下面这些方法：

```java
    @Override
    public void onSurfaceCreated(GL10 gl10, EGLConfig eglConfig) {
        // No depth test for 2D video.
        GLES30.glDisable(GLES30.GL_DEPTH_TEST);
        GLES30.glDepthMask(false);
        GLES30.glDisable(GLES30.GL_CULL_FACE);
        GLES30.glDisable(GLES30.GL_BLEND);

        GLES30.glActiveTexture(GLES30.GL_TEXTURE0);
        GLES30.glGenTextures(textures.length, textures, 0);
        GLES30.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, textures[0]);
        GLES30.glTexParameteri(
            GLES11Ext.GL_TEXTURE_EXTERNAL_OES,
            GLES30.GL_TEXTURE_MIN_FILTER,
            GLES30.GL_LINEAR
        );
        GLES30.glTexParameteri(
            GLES11Ext.GL_TEXTURE_EXTERNAL_OES,
            GLES30.GL_TEXTURE_MAG_FILTER,
            GLES30.GL_LINEAR
        );
        GLES30.glTexParameteri(
            GLES11Ext.GL_TEXTURE_EXTERNAL_OES,
            GLES30.GL_TEXTURE_WRAP_S,
            GLES30.GL_CLAMP_TO_EDGE
        );
        GLES30.glTexParameteri(
            GLES11Ext.GL_TEXTURE_EXTERNAL_OES,
            GLES30.GL_TEXTURE_WRAP_T,
            GLES30.GL_CLAMP_TO_EDGE
        );

        program = Utils.linkProgramGLES30(
            Utils.compileShaderResourceGLES30(
                context, GLES30.GL_VERTEX_SHADER, R.raw.vertex_30
            ),
            Utils.compileShaderResourceGLES30(
                context, GLES30.GL_FRAGMENT_SHADER, R.raw.fragment_30
            )
        );
        mvpLocation = GLES30.glGetUniformLocation(program, "mvp");
        // Position is set in shader sources.
        GLES30.glVertexAttribPointer(
            0, 2, GLES30.GL_FLOAT,
            false, 0, vertices
        );
        GLES30.glVertexAttribPointer(
            1, 2, GLES30.GL_FLOAT,
            false, 0, texCoords
        );

        GLES30.glEnableVertexAttribArray(0);
        GLES30.glEnableVertexAttribArray(1);
    }

    @Override
    public void onSurfaceChanged(GL10 gl10, int width, int height) {
        GLES30.glViewport(0, 0, width, height);
    }

    @Override
    public void onDrawFrame(GL10 gl10) {
        if (surfaceTexture == null) {
            return;
        }

        if (renderedFrame < updatedFrame) {
            surfaceTexture.updateTexImage();
            ++renderedFrame;
            // Utils.debug(
            //     TAG, "renderedFrame: " + renderedFrame + " updatedFrame: " + updatedFrame
            // );
        }

        GLES30.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        GLES30.glClear(GLES30.GL_COLOR_BUFFER_BIT);
        GLES30.glUseProgram(program);
        GLES30.glUniformMatrix4fv(mvpLocation, 1, false, mvp, 0);
        GLES30.glDrawElements(GLES30.GL_TRIANGLES, 6, GLES30.GL_UNSIGNED_INT, indices);
    }
```

这部份方法看名字就能看出来，需要注意的是那个 `gl10` 是 GLESv1 的用法，至于 v2 和 v3 都改成静态类了，所以我们用不到它。既然是 2D 贴图，就不需要深度测试和背面剔除了。然后设置纹理参数时需要用 `GLES11Ext.GL_TEXTURE_EXTERNAL_OES`，这个和 shader 里面是关联的。（哪个家伙把 GLESv2 和 v3 都支持的变量归属到 `GLES11Ext` 的，出来挨打！），别的地方会 OpenGL 的人都能看懂吧？最后，只有这三个函数有 OpenGL 上下文，其它的里面不能执行 OpenGL 操作。

绘图时候 **设置 mvp 矩阵不要用 nio 的 ByteBuffer**，虽然有一个多态函数可以用，但似乎那个函数不能工作，所以我直接使用了 float 数组。

下面是一些针对这个程序用到的方法。

```
    @Override
    void setSourcePlayer(@NonNull final SimpleExoPlayer exoPlayer) {
        // Re-create SurfaceTexture when getting a new player.
        // Because maybe a new video is loaded.
        createSurfaceTexture();
        exoPlayer.setVideoSurface(new Surface(surfaceTexture));
    }
```

这里是让播放器把解码的目标位置设置成 `Renderer` 里面和 OpenGL 纹理绑定的 `SurfaceTexture` 上，这个类是 Android 专门设置用来干这个的，可以阅读手册。

为什么是把播放器给 `Renderer` 让 `Renderer` 设置呢？因为 `Engine` 的生命周期里只用了一个 `Renderer` 但会经常释放重建 `Player`，每次都要重新建立对应视频大小的 `SurfaceTexture`，所以不能让 `Renderer` 暴露 `SurfaceTexture` 出去。

我这里没有用 `MediaPlayer`，而是用了 Google 的 `ExoPlayer` 这个项目，具体原因一会再说。

```
    @Override
    void setScreenSize(int width, int height) {
        if (screenWidth != width || screenHeight != height) {
            screenWidth = width;
            screenHeight = height;
            Utils.debug(TAG, String.format(
                Locale.US, "Set screen size to %dx%d", screenWidth, screenHeight
            ));
            updateMatrix();
        }
    }

    @Override
    void setVideoSizeAndRotation(int width, int height, int rotation) {
        // MediaMetadataRetriever always give us raw width and height and won't rotate them.
        // So we rotate them by ourselves.
        if (rotation % 180 != 0) {
            final int swap = width;
            width = height;
            height = swap;
        }
        if (videoWidth != width || videoHeight != height || videoRotation != rotation) {
            videoWidth = width;
            videoHeight = height;
            videoRotation = rotation;
            Utils.debug(TAG, String.format(
                Locale.US, "Set video size to %dx%d", videoWidth, videoHeight
            ));
            Utils.debug(TAG, String.format(
                Locale.US, "Set video rotation to %d", videoRotation
            ));
            updateMatrix();
        }
    }
```

因为要计算缩放，所以我这里设置了两个方法，分别获取屏幕尺寸和视频尺寸。然后只要在每次尺寸变化时候重新计算矩阵就行了，因为不是经常变化，所以就不用每次绘制都重新计算。

```
    @Override
    void setOffset(float xOffset, float yOffset) {
        final float maxXOffset = (1.0f - (
            (float)screenWidth / screenHeight) / ((float)videoWidth / videoHeight)
        ) / 2;
        final float maxYOffset = (1.0f - (
            (float)screenHeight / screenWidth) / ((float)videoHeight / videoWidth)
        ) / 2;
        if (xOffset > maxXOffset) {
            xOffset = maxXOffset;
        }
        if (xOffset < -maxXOffset) {
            xOffset = -maxXOffset;
        }
        if (yOffset > maxYOffset) {
            yOffset = maxYOffset;
        }
        if (yOffset < -maxXOffset) {
            yOffset = -maxYOffset;
        }
        if (this.xOffset != xOffset || this.yOffset != yOffset) {
            this.xOffset = xOffset;
            this.yOffset = yOffset;
            Utils.debug(TAG, String.format(
                Locale.US, "Set offset to %fx%f", this.xOffset, this.yOffset
            ));
            updateMatrix();
        }
    }
```

桌面在滑动的时候会调用 `Engine` 的方法告诉它滑动的值，为了实现视频随桌面平移的效果，我们可以在 `Engine` 里把值传给 `Renderer`，但我们要限制一下，偏移的位置不能超出视频尺寸，否则就出黑边了。

```
    private void createSurfaceTexture() {
        if (surfaceTexture != null) {
            surfaceTexture.release();
            surfaceTexture = null;
        }
        updatedFrame = 0;
        renderedFrame = 0;
        surfaceTexture = new SurfaceTexture(textures[0]);
        surfaceTexture.setDefaultBufferSize(videoWidth, videoHeight);
        surfaceTexture.setOnFrameAvailableListener(new SurfaceTexture.OnFrameAvailableListener() {
            @Override
            public void onFrameAvailable(SurfaceTexture surfaceTexture) {
                ++updatedFrame;
            }
        });
    }
```

需要注意这个函数里面的回调，为什么要设置一个计数器，而不是直接在这里执行 `SurfaceTexture.updateTexImage()` 呢？因为后者要求必须有 OpenGL 上下文……有上下文的只有之前的几个方法，其它是没有的。所以为什么这里用了两个计数器而不是一个 bool 变量呢（请自己回看 `onDrawFrame()` 函数）？因为这个 `SurfaceTexture` 内部似乎有排队机制，然后如果队列里有内容的话是不会调回调的……假如某一次传来了多于一帧，用 bool 变量翻转的时候就只能解决第一帧，后续视频就会卡住（这都是什么神仙写的 bug 啊喂！）。

```
    private void updateMatrix() {
        // Players are buggy and unclear, so we do crop by ourselves.
        // Start with an identify matrix.
        for (int i = 0; i < 16; ++i) {
            mvp[i] = 0.0f;
        }
        mvp[0] = mvp[5] = mvp[10] = mvp[15] = 1.0f;
        // OpenGL model matrix: scaling, rotating, translating.
        final float videoRatio = (float)videoWidth / videoHeight;
        final float screenRatio = (float)screenWidth / screenHeight;
        if (videoRatio >= screenRatio) {
            Utils.debug(TAG, "X-cropping");
            // Treat video and screen width as 1, and compare width to scale.
            Matrix.scaleM(
                mvp, 0,
                ((float)videoWidth / videoHeight) / ((float)screenWidth / screenHeight),
                1, 1
            );
            // Some video recorder save video frames in direction differs from recoring,
            // and add a rotation metadata. Need to detect and rotate them.
            if (videoRotation % 360 != 0) {
                Matrix.rotateM(mvp, 0, -videoRotation, 0,0, 1);
            }
            Matrix.translateM(mvp, 0, xOffset, 0, 0);
        } else {
            Utils.debug(TAG, "Y-cropping");
            // Treat video and screen height as 1, and compare height to scale.
            Matrix.scaleM(
                mvp, 0, 1,
                ((float)videoHeight / videoWidth) / ((float)screenHeight / screenWidth), 1
            );
            // Some video recorder save video frames in direction differs from recoring,
            // and add a rotation metadata. Need to detect and rotate them.
            if (videoRotation % 360 != 0) {
                Matrix.rotateM(mvp, 0, -videoRotation, 0,0, 1);
            }
            Matrix.translateM(mvp, 0, 0, yOffset, 0);
        }
        // This is a 2D center crop, so we only need model matrix, no view and projection.
    }
}
```

最后是计算矩阵，这里也很简单，只要注意计算时候，要分别以视频和屏幕最长的一条边作为单位一，然后分别缩放两者的另一侧，只是数学计算而已。然后为什么会有旋转呢？如果你不写旋转而又使用手机拍摄的竖屏视频做壁纸，就会发现方向是旋转了 90 度的，所以宽和高也都错了。原因是一些设备录像时候不会旋转帧内容，而是以传感器原生的方向存放像素，然后在视频 Metadata 里面记录一下旋转角度，由播放器做旋转。`MediaPlayer` 自己直接输出是会旋转的，但由于我们这里用它做解码器，所以它传过来的帧是原样的。

解决方法是在 `Engine` 加载视频的时候先读取一下元数据，从元数据里获取视频的尺寸旋转量设置给 `Renderer`：

```java
        private void getVideoMetadata() throws IOException {
            final MediaMetadataRetriever mmr = new MediaMetadataRetriever();
            switch (wallpaperCard.getType()) {
            case INTERNAL:
                final AssetFileDescriptor afd = getAssets().openFd(wallpaperCard.getPath());
                mmr.setDataSource(
                    afd.getFileDescriptor(),
                    afd.getStartOffset(),
                    afd.getDeclaredLength()
                );
                afd.close();
                break;
            case EXTERNAL:
                mmr.setDataSource(context, wallpaperCard.getUri());
                break;
            }
            final String rotation = mmr.extractMetadata(
                MediaMetadataRetriever.METADATA_KEY_VIDEO_ROTATION
            );
            final String width = mmr.extractMetadata(
                MediaMetadataRetriever.METADATA_KEY_VIDEO_WIDTH
            );
            final String height = mmr.extractMetadata(
                MediaMetadataRetriever.METADATA_KEY_VIDEO_HEIGHT
            );
            mmr.release();
            videoRotation = Integer.parseInt(rotation);
            videoWidth = Integer.parseInt(width);
            videoHeight = Integer.parseInt(height);
        }
```

当然 `WallpaperCard` 是我自己封装的类，关系不大。总之是使用 `MediaMetadataRetriever` 就可以了。至于你说视频元数据和实际内容不符怎么办？那是用户问题，用户自己处理不好视频为什么要我来解决？随便让他拿电脑上修改一下好了。

这些实际上是这个应用的核心内容，其它的播放之类的都大同小异，总之如果你实现了 OpenGL 的剪裁功能，应该就可以正常播放壁纸了。其它部分直接参考我的代码或者网上的样例就可以了。

顺便说一下，`SurfaceTexture` 似乎会给一个纹理矩阵，但我试着用了一下发现并不是需要的效果……所以还是自己处理吧。

# 然后声音又不对劲了……

就算你给 `MediaPlayer` 设置了静音，还是不太对劲！正常来说在桌面按音量键设置的是铃声音量，为什么变成了媒体音量！

这当然是因为虽然 `MediaPlayer` 虽然静音了，但还是占据着音频通道，我们都知道 Android 分为媒体、铃声、系统等音量通道，如果 `MediaPlayer` 一直占着媒体通道，其它的媒体 App 比如音乐播放器一类的就没有办法播放。

使用 `MediaPlayer` 是没有解决办法的，因为它没提供关闭音轨的接口，一开始我差点基于 `MediaCodec` 自己编写一个视频播放器，但始终没办法把解码器从主线程转移开，而且 CPU 占用率很高。后来发现 `ExoPlayer` 有办法关闭音轨，所以我换成了 `ExoPlayer`。

具体怎么引入 `ExoPlayer` 请参照官方文档，只要引入 `core` 部分就可以了。

我这里给出一段用于关闭音轨的代码，网上不太好找：

```java
            trackSelector = new DefaultTrackSelector();
            exoPlayer = ExoPlayerFactory.newSimpleInstance(context, trackSelector);
            exoPlayer.setVolume(0.0f);
            // Disable audio decoder.
            final int count = exoPlayer.getRendererCount();
            for (int i = 0; i < count; ++i) {
                if (exoPlayer.getRendererType(i) == C.TRACK_TYPE_AUDIO) {
                    trackSelector.setParameters(
                        trackSelector.buildUponParameters().setRendererDisabled(i, true)
                    );
                }
            }
```

主要是创建 Player 时候单独给它一个 DefaultTrackSelector。然后关掉音轨。

这样大概就可以运行了，剩下的一些问题都和视频渲染无关。

# 写 UI 也很麻烦！

虽然我知道你肯定不满足只使用内嵌视频，但如果你像我一样写了个可以设置其它视频的 UI，就会遇到各种各样的问题。写 UI 的时候，你不是在编写回调函数，就是在调用回调函数的路上。

最令人恶心的是 `Context` 管理，很多地方都需要 `Context`，但很多地方都没法传 `Context`，比如 Android 推荐你用 `Uri` 而不是直接用文件路径，但所有使用 `Uri` 的地方都要求你传 `Context` 来解析 `Uri`！比如我添加视频，需要解析 `Uri` 并生成缩略图，为了不阻塞线程，必须开启一个 `AsyncTask`，但是，`AsyncTask` 不能直接拿走 `MainActivity` 作为自己的 `Context`！必须要传弱引用，总之没有一个优雅的解决方案。

再比如你需要一些在各个位置都能访问到的数据，你可以自己写一个 `Application` 类并给它静态变量，但是静态和动态不能互相交叉，也就是说你不能封装一个操作数据的同时需要访问 `Context` 的方法，所以很多地方就只能拆开写，没办法封装。

以及列表和数据的绑定写起来真的很麻烦，说实在的，HTML 和 JavaScript 写 UI 真的比 Android 和 Java 舒服的多……

顺便，你可能会想在 UI 里调用系统的动态壁纸预览，然后根据它返回的结果来设置选中的视频，但这个真的很不靠谱，比如某些版本的 MIUI 修改的预览器即使你点击了应用，它仍然会返回一个取消的结果。明明是 MIUI 的问题，却需要我来解决，我大概可以理解为什么 sway 的开发者大骂 NVIDIA 用户了。你们给了小米钱买它的手机和系统，然后小米造成的问题要我来解决，我一分钱都没有拿到，哪里来的义务和责任解决？？？

当然我最后还是想办法绕开了，添加了一个单独的应用按钮，用户点击这个按钮就是忽略预览，直接设置为壁纸。

动态壁纸预览器调用你的壁纸服务的时候，你的 `Engine` 里 `isPreview()` 方法会返回 `true`，可以根据这个判断是不是在预览状态。因为我只有一个壁纸服务，所以我做了一些判断，对预览和桌面分别做了一些不同了逻辑，以便防止预览的结果影响到本来桌面上的壁纸效果。

最后要注意：你可以在 `Service` 里面访问 `Application` 的静态方法，但似乎你不能一直依靠这个。比如当你设置壁纸以后重启了，`Service` 会被启动，但这时似乎没办法访问到 `Application` 里的变量。我解决的办法是将数据写入到 `SharedPreference`，如果从 `Application` 加载失败就去读取 `SharedPreference`，但总之我没找到有关 `Service` 和 `Application` 会不会一起创建的说明，所以这部份的代码写的很混乱（也包含上面说的不能封装的原因）。

# 一杯咖啡都没有……

如果你只是觉得市面上所有的视频动态壁纸程序都太臃肿太不可靠（包括抖音和火萤）而想自己写一个，我建议你还是放弃，因为遇到的坑比你想象的多，解决起来也很麻烦。而且实际上，你完全可以使用我写好的程序。

这个程序只有 3.06M，支持加载本地壁纸，支持平板电脑，支持壁纸滑动，不显示时自动暂停播放节约电量，严格遵循 Android 规范，没有任何敏感权限（比如联网和操作外置存储），不请求文件真实路径，测试下来功能基本没有问题。如果你还不放心，所有的代码都在 GitHub 上，我不怕你说我代码写的不好，你大可以看一遍有没有后门。

你可以分别从 [GitHub Release](https://github.com/AlynxZhou/AlynxLiveWallpaper/releases/latest)，[Google Play](https://play.google.com/store/apps/details?id=xyz.alynx.livewallpaper) 和 [酷安网](https://www.coolapk.com/apk/xyz.alynx.livewallpaper) 下载这个应用。

为了上架 Google Play 我支付了 25 美元申请了开发者账户，应用是免费的，但我放置了捐赠页面。不过已经有几百个下载量（大部分是酷安）之后，我还没有从这个应用里得到一点回报，如果你觉得应用不错并且手头宽裕，可以打开 App 赞助我一杯咖啡。毕竟解决上面提到的所有的坑并不容易。

*Alynx Zhou*

**A Coder & Dreamer**
