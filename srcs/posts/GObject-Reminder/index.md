---
title: GObject 备忘录
layout: post
createdDate: 2020-09-05 09:23:00
categories:
- 计算机
- 编程
tags:
- GObject
---
说来惭愧，我一直记不太清 GObject 到底是怎么用的，毕竟作为一个写过 C++ 和 Python 然后常用 JS 的人来说，面向对象的实现是非常自然的，不需要考虑为什么。所以我总是看着一大堆类型转换和分散的定义以及各种 chain up 感到眩晕。而 GObject 的文档写的也相当分散，有种管中窥豹之感。

<!--more-->

有同学觉得其实把函数变成指针放在结构体里看起来就面向对象了，以前我也这么觉得，但是显然这样不能实现继承封装等等特性，GObject 独特之处在于它实现了这些，并且是和语言独立的。当然，要想在一个没有这些概念的语言里面做到这些，就有很多需要自己手动处理和函数库处理的地方，就是因为有些隐藏了起来有些又要自己做，才让它看起来像古老的黑魔法。

最近我尝试做了一个小项目，以便搞清楚如何按照 GObject 的模式编写一些继承封装的代码。

首先得给项目起名字，因为 GObject 要求你的命名符合一定的约定，其中一个前缀是项目名，不过这个 简单，就叫 `test` 好了，然后做一个基类叫 `animal`，于是就有了 `test-animal.h`:

```c
#ifndef __TEST_ANIMAL_H__
#define __TEST_ANIMAL_H__

#include <glib-object.h>

G_BEGIN_DECLS

// 这个玩意必须要手动定义。
#define TEST_TYPE_ANIMAL test_animal_get_type()
// 这个玩意会展开成一大堆函数声明、typedef 什么的，
// 所以我们只要按约定定义结构体实现函数。
G_DECLARE_DERIVABLE_TYPE(TestAnimal, test_animal, TEST, ANIMAL, GObject)

// 但是上面那个玩意其实不会给你搞一个类结构体出来，
// 而且这个因为是给其他文件用的所以必须写在头文件里，
// 不然人家怎么知道你有什么虚函数！
struct _TestAnimalClass {
	GObjectClass parent_class;
	// 定义一个可以继承的函数。
	void (*print)(TestAnimal *animal);
	gpointer padding[12];
};

// 因为上面的 class 里面定义了，这个只是调用那个。
void test_animal_print(TestAnimal *animal);
// 这个玩意得手动定义，是个不可继承的公开函数。
TestAnimal *test_animal_new(gchar *animal_name);

G_END_DECLS

#endif
```

因为代码里写了很多注释所以我这里就不再啰嗦一遍了，说点里面没写的。

`G_DECLARE_DERIVABLE_TYPE` 表示你声明了一个可以继承的类，也就是说你需要自己弄一个虚函数表出来。有时候你会在某个 GObject 的项目里定义了一大堆宏（比如 GTK 就不爱用这个而是手动定义），其实它们和 `G_DECLARE_DERIVABLE_TYPE` 做了一样的工作，因为总要做，就写了个宏实现。这个并不会给你定义具体的类结构体（其实就是虚函数表，用来存放所有可以继承重载的函数），所以我们要按照约定自己写一个 `_TestAnimalClass` 的结构体，在类型名字前面加下划线作为结构体名也是约定俗成的，`G_DECLARE_DERIVABLE_TYPE` 会展开出一句 `typedef struct _TestAnimalClass TestAnimalClass`。

这个类结构体约定第一个元素是它的父类型的类结构体——这其实意味着我们复制了一份父类型的虚函数表出来，于是我们就可以覆盖父类型的方法而不修改原本的父类型。这一句可能比较难懂，不过后面还有关联。

这个类结构体存放的并不是实例的变量，它有点类似于 JS 里面的原型，这样我们就不需要给每一个生成的实例复制一份虚函数了，它们共用一个虚函数表。

然后会有一个 `test-animal.c`:

```c
#include "test-animal.h"

// Derivable 类型会自动帮你定义实例结构体，
// 所以你想夹带点私货就得自己搞个 Private 类型。
typedef struct {
	gchar *animal_name;
} TestAnimalPrivate;

// 这个也会展开一大堆声明什么的。
G_DEFINE_TYPE_WITH_PRIVATE(TestAnimal, test_animal, G_TYPE_OBJECT)

// 要想通过 new 函数直接初始化一些值就需要搞点属性。
enum {
	PROP_0,
	PROP_ANIMAL_NAME,
	N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = {NULL};

static void set_property_impl(
	GObject *object,
	guint property_id,
	const GValue *value,
	GParamSpec *pspec
)
{
	TestAnimal *animal = TEST_ANIMAL(object);
	TestAnimalPrivate *priv = test_animal_get_instance_private(animal);

	switch (property_id) {
	case PROP_ANIMAL_NAME:
		if (priv->animal_name)
			g_free(priv->animal_name);
		// 所以其实我们是在设置属性的时候更新私有成员。
		priv->animal_name = g_value_dup_string(value);
		break;
	default:
		/* We don't have any other property... */
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void get_property_impl(
	GObject *object,
	guint property_id,
	GValue *value,
	GParamSpec *pspec
)
{
	TestAnimal *animal = TEST_ANIMAL(object);
	TestAnimalPrivate *priv = test_animal_get_instance_private(animal);

	switch (property_id) {
	case PROP_ANIMAL_NAME:
		// 所以你看属性这个名称和我们想的属性不一样，
		// 其他语言里面属性就是成员，能存点东西，
		// 但这里好像属性只是成员的一个代理，
		// 具体的东西存在成员里面，通过属性设置。
		g_value_set_string(value, priv->animal_name);
		break;
	default:
		/* We don't have any other property... */
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

// 给子类提供一个默认的实现，这样它就不是一个纯虚函数了。
static void print_impl(TestAnimal *animal)
{
	TestAnimalPrivate *priv = test_animal_get_instance_private(animal);
	g_message("I am an Animal called %s.", priv->animal_name);
}

static void test_animal_class_init(TestAnimalClass *animal_class)
{
	// 首先我们覆盖这个类里面的 GObject 类的方法，
	// 你调用 GObject 的函数，他会先获取参数的 GObject 类，
	// 那就获取到我们这个了，然后具体的实现就是我们覆盖的这个。
	GObjectClass *object_class = G_OBJECT_CLASS(animal_class);
	object_class->get_property = get_property_impl;
	object_class->set_property = set_property_impl;

	// 给虚方法设置默认实现。
	animal_class->print = print_impl;

	obj_properties[PROP_ANIMAL_NAME] = g_param_spec_string(
		"animal-name",
		"Animal Name",
		"Name of Animal",
		NULL,
		G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE
	);

	g_object_class_install_properties(
		object_class,
		N_PROPERTIES,
		obj_properties
	);
}

static void test_animal_init(TestAnimal *animal)
{
}

void test_animal_print(TestAnimal *animal)
{
	// 你看其实这里和 GObject 类设置属性的原理是一样的。
	// 子类调用这个函数，实际上获取到的是子类自己那个 Animal 类，
	// 于是他只要覆盖自己的那个 Animal 类，调用的就是他自己的。
	TestAnimalClass *animal_class;
	animal_class = TEST_ANIMAL_GET_CLASS(animal);
	g_return_if_fail(animal_class->print != NULL);
	animal_class->print(animal);
}

TestAnimal *test_animal_new(gchar *animal_name)
{
	g_return_val_if_fail(animal_name != NULL, NULL);
	return g_object_new(
		TEST_TYPE_ANIMAL,
		"animal-name", animal_name,
		NULL
	);
}
```

理论上来说我们直接往实例结构体里添加成员作为私有成员就可以，但是在头文件里定义实例结构体，其它文件是可以看到实例结构体的成员的，所以 `G_DECLARE_DERIVABLE_TYPE` 不允许我们这么做。

但有一个省事的宏 `G_DEFINE_TYPE_WITH_PRIVATE`，它要求我们做一个 Private 类型，里面存放我们想要的任何私有元素，它不会被继承，并且因为定义在 `.c` 文件里所以也不会被其它文件看到。它还会提供一些 GObject 内置类型系统的实现函数（不然 GObject 系统怎么知道你建了哪些类型！），还有一些只在实现里面用到的定义。当然 Private 类型的名字也是约定好的。

多说一句，按照 GTK 的代码实现，其实 Private 最后就是在实例结构体里面定义了一个叫 `priv` 的指针，类型是自己定义的 Private 类型，因为 GTK 是手写的头文件定义然后用的 `G_DEFINE_TYPE_WITH_PRIVATE` 同时又没产生冲突，其实你也可以完全不理会这两个宏，全都自己写，只要保证类结构体和实例结构体的第一个成员是父类结构体和父实例结构体就可以了。但是 `G_DEFINE_TYPE_WITH_PRIVATE` 提供了一个 `项目名_类型名_get_instance_private` 的函数，我们就不用直接访问 `priv` 成员了。

在使用 `g_object_new` 新建一个对象的时候可以通过 key-value 的方式设置一些初始值，这个其实是通过 GObject 提供的 property 功能实现的，因为 C 并没有哈希表这种东西。这个过程很繁琐但也很固定，其实就是实现其它语言里面传一个对象作为构造函数参数然后以此设置私有成员初始值的功能。property 就是参数，具体的内容还是存在初始值里面的，不过其实你也可以通过 property 访问具体的值。这里很容易理解成 property 有自己单独的存储空间，其实不是。property 是可以继承的，所以子类可以同时设置父类和自己的 property。

然后我们要接触到第一个重载的部分了，因为 `TestAnimal` 继承了 GObject，所以我们要重载掉它类结构体里面的父类结构体的 `set_property` 和 `get_property` 函数。过程也很简单，GObject 要求我们实现两个函数 `项目名_类型名_class_init` 和 `项目名_类型名_class_init`，其中前者就是让我们初始化类结构体用的。首先进行一个类型转换把 `TestAnimalClass` 转换成 `GObjectClass`（为什么可以强转？那你先思考一下为什么定义类结构体时候第一个元素是父类结构体？就是因为要这样才能进行类型转换，本质上是个套娃），然后直接赋值。这样假如有人对我们这个类型执行 `g_object_set_property`，实际上是调用的我们重载过的函数（为什么？怎么做到的？往下看）。

那其实我们知道如何覆盖父类的方法，但运行的时候是如何动态重载到我们自己的函数的也不清楚，我们先搞定我们自己的那个虚函数，其实很简单，我们定义具体的方法的时候（指 `test_animal_print`），不要让它实现具体的逻辑，而是让它通过参数的实例执行虚函数表里的函数就可以了。这需要一个自动生成的 `项目名_类型名_GET_CLASS` 的宏，它的作用是通过一个实例查找到 **这个实例本身** 对应的类结构体，然后就可以运行虚函数了。比如我给 `g_object_set_property` 传一个 `TestAnimal`，那我们调用的其实是 `TestAnimalClass` 的第一个成员那个 `GObjectClass` 的 `set_property`，这个已经被我们改成自己的了，于是就实现了一个 ~~不那么直观的~~ 重载。

当然假如我们不想写一个纯虚函数，可以在 `项目名_类型名_class_init` 里面设置一个初值，这样假如子类没有重载，调用的就是这个。

那你可能要问假如我有一个不想被重载的函数呢，那你就不要跳虚函数表了，直接写逻辑就可以了。

接下来我们终于可以写子类的，首先就是 `test-cat.h`:

```c
#ifndef __TEST_CAT_H__
#define __TEST_CAT_H__

#include <glib-object.h>
#include "test-animal.h"

G_BEGIN_DECLS

#define TEST_TYPE_CAT test_cat_get_type()
// Final 类型就不用写类结构体啦，反正那个最大的用处是用来写可以继承的虚函数。
G_DECLARE_FINAL_TYPE(TestCat, test_cat, TEST, CAT, TestAnimal);

TestCat *test_cat_new(gchar *animal_name, gchar *cat_name);

G_END_DECLS

#endif
```

`TestCat` 是继承 `TestAnimal` 的，并且我们不想让它被继承，所以它就不需要写类结构体了（因为虚函数表是用来重载的，没有继承当然也没有重载）。

所以接下来就直接到实现部分了，在 `test-cat.c`:

```c
#include "test-cat.h"

// Final 类型也没有 Private，但是它自己本身就是容器嘛！
// 所以实例结构体就给你自己写了。
// 当然别的文件不需要知道你有什么私货所以不要写在头文件里。
struct _TestCat {
	TestAnimal parent_instance;
	gchar *cat_name;
};

G_DEFINE_TYPE(TestCat, test_cat, TEST_TYPE_ANIMAL)

enum {
	PROP_0,
	PROP_CAT_NAME,
	N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = {NULL};

static void set_property_impl(
	GObject *object,
	guint property_id,
	const GValue *value,
	GParamSpec *pspec
)
{
	TestCat *cat = TEST_CAT(object);

	switch (property_id) {
	case PROP_CAT_NAME:
		if (cat->cat_name)
			g_free(cat->cat_name);
		cat->cat_name = g_value_dup_string(value);
		break;
	default:
		/* We don't have any other property... */
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void get_property_impl(
	GObject *object,
	guint property_id,
	GValue *value,
	GParamSpec *pspec
)
{
	TestCat *cat = TEST_CAT(object);

	switch (property_id) {
	case PROP_CAT_NAME:
		g_value_set_string(value, cat->cat_name);
		break;
	default:
		/* We don't have any other property... */
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

// 那我们不想要默认的实现就可以覆盖掉自己的。
static void print_impl(TestAnimal *animal)
{
	TestCat *cat = TEST_CAT(animal);
	g_message("I am a Cat called %s.", cat->cat_name);
	// 这个自动生成的 parent 变量指向的是真正的父类！
	// 因为我们自己带的父类结构体被我们覆盖了，所以这里给你一个访问正主的机会。
	// 它是个 gpointer 所以你得手动设置类型。
	// 其实你的子类如果是完全覆盖父类的功能其实我觉得不用写这一句了。
	// 但是有时候你的子类是在父类上面增加功能就得写。
	// 这不就是 super 指针嘛！
	TEST_ANIMAL_CLASS(test_cat_parent_class)->print(animal);
}

static void test_cat_class_init(TestCatClass *cat_class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(cat_class);
	object_class->get_property = get_property_impl;
	object_class->set_property = set_property_impl;

	// 干掉我们自己的这个父类结构体里面带的内置实现，
	// 这样父类的函数调用的其实是我们自己的这个实现。
	// 那你看你一会让父类调用自己一会又让自己调用父类可真是麻烦。
	TestAnimalClass *animal_class = TEST_ANIMAL_CLASS(cat_class);
	animal_class->print = print_impl;

	obj_properties[PROP_CAT_NAME] = g_param_spec_string(
		"cat-name",
		"Cat Name",
		"Name of Cat",
		NULL,
		G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE
	);

	g_object_class_install_properties(
		object_class,
		N_PROPERTIES,
		obj_properties
	);
}

static void test_cat_init(TestCat *cat)
{
}

TestCat *test_cat_new(gchar *animal_name, gchar *cat_name)
{
	g_return_val_if_fail(animal_name != NULL && cat_name != NULL, NULL);
	return g_object_new(
		TEST_TYPE_CAT,
		"animal-name", animal_name,
		"cat-name", cat_name,
		NULL
	);
}
```

Final 类型没有 Private，因为它自己就是 Private，所以这个定义实例结构体的权力交给了你，直接在里面写私有成员即可，但是不要忘了套娃（指第一个成员一定要是父实例结构体）。

然后就是设置我们自己的 property 并重载 GObject 的虚函数，当然你可能会说这样父类型的 property 不就丢了吗？其实 property 是放在 GObject 系统单独的一个表里，需要的时候它去查询，并且会考虑父类型的继承链，所以我们不需要考虑重载时候覆盖掉父类型的问题。

接下来就是重载父类型提供给我们的虚函数也就是 `TestCatClass.print` 了！当然首先是把我们自己的类结构体里面的父类结构体的虚函数换掉。

在写我们自己重载的函数的时候，一个比较重要的事情是 chain up，也就是最后一行那个 `TEST_ANIMAL_CLASS(test_cat_parent_class)->print(animal);`，有时候你重载只是做了一些子类自己的处理，然后还是需要父类提供的处理函数的，这该怎么办？你可能会说类结构体里面不是有个父类结构体了吗？但那个已经被我们覆盖了！你这样不就是咬自己尾巴的猫吗？

当我们自己解决不了的时候就得 GObject 解决了，`G_DEFINE_TYPE` 会提供一个 `项目名_类型名_parent_class` 的指针定义，这个指向的是那个真正的唯一的 **不是我们自己包含的** 那个父类结构体！也就是说对于子类和父类继承而言，有两个父类结构体，一个是父类自己作为一个类的那个，另一个是子类为了实现重载包含的那个，这个访问的就是前者。

当然这个类型其实是个 `gpointer` 所以我们还得自己转换一下类型，然后只要调用方法就可以了（这不就是 super 指针嘛）。

最后我们写一个测试的程序 `main.c`:

```c
#include "test-animal.h"
#include "test-cat.h"

int main(int argc, char *argv[])
{
	TestAnimal *animal = test_animal_new("animal1");
	TestCat *cat = test_cat_new("animal2", "cat1");
	test_animal_print(animal);
	test_animal_print(TEST_ANIMAL(cat));
	return 0;
}
```

然后用这个 `Makefile`:

```
CC := cc
OBJECTS := main.o test-animal.o test-cat.o
CFLAGS := `pkg-config --cflags gobject-2.0`
LIBS := `pkg-config --libs gobject-2.0`

all: ${OBJECTS}
	${CC} -o test ${OBJECTS} ${LIBS}

main.o: test-animal.h test-cat.h
	${CC} -c main.c ${CFLAGS}

test-animal.o:
	${CC} -c test-animal.c ${CFLAGS}

test-cat.o: test-animal.h
	${CC} -c test-cat.c ${CFLAGS}

.PHONY: clean
clean:
	-rm test ${OBJECTS}
```

运行的结果是这样的：

```
** Message: 11:23:25.727: I am an Animal called animal1.
** Message: 11:23:25.729: I am a Cat called cat1.
** Message: 11:23:25.729: I am an Animal called animal2.
```

你可以看到第一行是父类也就是 `TestAnimalClass` 的函数的输出。第二行是 `TestCatClass` 重载的函数的输出，第三行则是 `TestCatClass` 重载的函数 chain up 到 `TestAnimalClass` 的输出，但是输出的是子类继承的父类的属性！

最后有关我到底该声明成子类还是父类以及传参时候要不要类型转换：其实在这个 GObject 的样例里面看起来是无所谓的，类型转换可行不可行其实取决于具体 new 的是什么，而不是声明的指针是什么，所以其实声明成父类和子类都没有关系，类型转换也主要是为了满足 C 语言对指针类型和函数声明的要求罢了。

*Alynx Zhou*

**A Coder & Dreamer**
