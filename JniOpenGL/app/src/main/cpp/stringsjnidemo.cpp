#include <jni.h>
#include <string>
#include <random>
#include <iostream>
#include <android/log.h>

/**
 * These will be the first adjective, with a -ly added to modify the second adjective
 */
std::string adjective1Corpus[] = {
        "Abaft",
        "Abandoned",
        "Abased",
        "Abashed",
        "Abasic",
        "Abbatial",
        "Abdicable",
        "Abdicant",
        "Abdicative",
        "Abdominal",
        "Abdominous",
        "Abducent",
        "Aberrant",
        "Aberrational",
        "Abeyant",
        "Abhorrent",
        "Abiotic",
        "Ablaze",
        "Able",
        "Ablebodied",
        "Ablutophobic",
        "Abnormal",
        "Abolitionary",
        "Abominable",
        "Aboriginal",
        "Above",
        "Aboveground",
        "Abrupt",
        "Absent",
        "Absentminded",
        "Absolute",
        "Absolutistic",
        "Abstract",
        "Abstracted",
        "Absurd",
        "Abusive",
        "Abysmal",
        "Abyssal"
};
/**
 * These will be the primary adjectives to modify the animal
 */
std::string adjective2Corpus[] = {
        "Nyctophobic",
        "Nylon",
        "OAFISH",
        "OBEDIENT",
        "OBELISKOID",
        "OBESE",
        "OBJECTIVE",
        "OBLIVIOUS",
        "OBLONG",
        "OBNOXIOUS",
        "OBSCENE",
        "OBSEQUIOUS",
        "OBSERVANT",
        "OBSESSIVE",
        "OBSIDIAN",
        "OBSOLETE",
        "OBTUSE",
        "OBVIOUS",
        "OCCASIONAL",
        "OCCUPATIONAL",
        "OCEANGOING",
        "OCEANIC",
        "OCEANLIKE",
        "OCEANOGRAPHIC",
        "OCEANOGRAPHICAL",
        "OCHRE",
        "OCTAGONAL"
};
/**
 * Animals to be described
 */
std::string animalCorpus[] = {
        "maltesedog",
        "mamba",
        "mamenchisaurus",
        "mammal",
        "mammoth",
        "manatee",
        "mandrill",
        "mangabey",
        "manta",
        "mantaray",
        "mantid",
        "mantis",
        "mantisray",
        "manxcat",
        "mara",
        "marabou",
        "marbledmurrelet",
        "mare",
        "marlin",
        "marmoset",
        "marmot",
        "marten",
        "martin",
        "massasauga",
        "massospondylus",
        "mastiff",
        "mastodon",
        "mayfly",
        "meadowhawk",
        "meadowlark",
        "mealworm",
        "meerkat",
        "megalosaurus",
        "megalotomusquinquespinosus",
        "megaraptor",
        "merganser",
        "merlin",
        "metalmarkbutterfly",
        "metamorphosis",
        "mice",
        "microvenator",
        "midge",
        "milksnake",
        "milkweedbug",
        "millipede",
        "minibeast",
        "mink",
        "minnow",
        "mite",
        "moa",
        "mockingbird",
        "mole",
        "mollies",
        "mollusk",
        "molly",
        "monarch",
        "mongoose",
        "mongrel"
};
std::string testStrings[] = {"hello","world"};
extern "C" JNIEXPORT jstring JNICALL
Java_com_jeffcreswell_jniopengl_jni_JniHooks_randomString(
        JNIEnv *env,
        jobject thiz) {

    // random value from 0 to the size of our text corpi, which is calculated
    // as the entire size of each array divided by the size of String object

    //Will be used to obtain a seed for the random number engine
    std::random_device rd;

    // mersenne_twister_engine seeded with rd()
    std::mt19937 randomIndexGenerator(rd());

    // uniform distributions from 0 to array length-1 for each array
    std::uniform_int_distribution<> randomAdj1IndexDistribution(0,
            (sizeof(adjective1Corpus) / sizeof(std::string))-1);
    std::uniform_int_distribution<> randomAdj2IndexDistribution(0,
            (sizeof(adjective2Corpus) / sizeof(std::string))-1);
    std::uniform_int_distribution<> randomAnimalIndexDistribution(0,
            (sizeof(animalCorpus) / sizeof(std::string))-1);

    // range of string count
    std::uniform_int_distribution<> randomStringCountDistribution(1,3);

    // generate a random index for each array
    int adj1Index = randomAdj1IndexDistribution(randomIndexGenerator);
    int adj2Index = randomAdj2IndexDistribution(randomIndexGenerator);
    int animalIndex = randomAnimalIndexDistribution(randomIndexGenerator);

    // choose randomly between 1 and 3 strings
    int stringCount = randomStringCountDistribution(randomIndexGenerator);

    // concat randomly selected strings into a random silly phrase
    // that includes at least then noun and up to 2 adjectives.
    // Naturally, the code doesn't work without the animal (or, at least,
    // it shouldn't).
    std::string animalWorthyOfWindyDescription;
    if(stringCount == 3){
        animalWorthyOfWindyDescription = adjective1Corpus[adj1Index] +
                "ly " + adjective2Corpus[adj2Index] + " " +
                animalCorpus[animalIndex];
    }else if(stringCount == 2){
        animalWorthyOfWindyDescription = adjective2Corpus[adj2Index] +
                " " + animalCorpus[animalIndex];
    }else{
        animalWorthyOfWindyDescription = animalCorpus[animalIndex];
    }

    // native logcat API logging
    __android_log_print(ANDROID_LOG_DEBUG, "jnigldemo", "JNI returning random adj: %s",
            animalWorthyOfWindyDescription.c_str());

    // return jstring of our result std::string to Java
    return env->NewStringUTF(animalWorthyOfWindyDescription.c_str());
}

/*
extern "C" JNIEXPORT void JNICALL
Java_com_jeffcreswell_jniopengl_jni_JniHooks_testString(
        JNIEnv *env,
        jobject ) {
*/

    // todox: whelp, apparently the NDK's hello world example crashes... good old JNI.
    // Anyway, it looks like the workaround is to receive/create a jstring and then
    // call env->GetStringUTFChars(jstr, 0) to get the modified UTF 8 encoded version as char*
    // and then create a std::string over that char*.  THEN we should be able do the NewStringUTF() shuffle.
    // Alternatively, either receive the string from Java so we don't have to worry about encoding
    // or create and return a jobject that is actually a java.land.String based on the actual string data
    // we want to use.  Even that requires GetStringUTFChars() over the encoding string, though, so
    // there's not really a nice solution AFAICS
    // UPDATE: it turns out my app was crashing over an attempt to access the as-yet unimplemented GlSurfaceView.
    // The myriad NewStringUTF crash logs were presumably from other processes.  I'm nervous about the whole
    // expected input is modified UTF-8 thing now, though, so to be safe I may just go ahead with providing
    // jstring array corpus arg with pre-javified strings and then do the interesting bits here.
    // TODO: why were all those other crashes present, though?  Were they all trying to use supplementary
    // unicode chars (U+10000 and above)?

    /* // crash over "JNI DETECTED ERROR IN APPLICATION: use of invalid jobject"?
    std::string helloString = "hello strings from JNI";
    jstring javifiedString = env->NewStringUTF(helloString.c_str());
    const char* actualString = env->GetStringUTFChars(javifiedString,nullptr);
    __android_log_print(ANDROID_LOG_DEBUG, "jnigldemo", "NewStringUTF successfully made java string: %s",
                        actualString);
    env->ReleaseStringUTFChars(javifiedString,actualString);
    */


    /*
    // also explodes when we try NewStringUTF just to get the character encoding string
    std::string helloString = "hello strings from JNI";
    jbyteArray array = env->NewByteArray(helloString.size());
    env->SetByteArrayRegion(array, 0, helloString.size(), (const jbyte*)helloString.c_str());
    jstring strEncode = env->NewStringUTF("UTF-8");
    jclass cls = env->FindClass("java/lang/String");
    jmethodID ctor = env->GetMethodID(cls, "<init>", "([BLjava/lang/String;)V");
    jstring object = (jstring) env->NewObject(cls, ctor, array, strEncode);
    return object;
    */

    /* // wheee!  This works a treat.  However, relying on system default charset seems less than ideal.
    std::string helloString = "hello strings from JNI";
    jbyteArray array = env->NewByteArray(helloString.size());
    env->SetByteArrayRegion(array, 0, helloString.size(), (const jbyte*)helloString.c_str());
    jclass cls = env->FindClass("java/lang/String");
    jmethodID ctor = env->GetMethodID(cls, "<init>", "([B)V");
    jstring object = (jstring) env->NewObject(cls, ctor, array);
    return object;
    */
/*
}
*/


