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
    // todo: choose randomly from adj1, adj2, and animal corpi and then combine for the result string

    // random value from 0 to the size of our adjective1 corpus, which is calculated
    // as the entire size of the array divided by the size of String object
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 randomIndexGenerator(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> randomIndexDistribution(0,
            (sizeof(testStrings) / sizeof(std::string))-1);
    int adj1Index = randomIndexDistribution(randomIndexGenerator);
    __android_log_print(ANDROID_LOG_DEBUG, "jnigldemo", "JNI returning random adj: %s",
            testStrings[adj1Index].c_str());
    std::string hello = "hello world";
    return env->NewStringUTF(hello.c_str());
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_jeffcreswell_jniopengl_jni_JniHooks_testString(
        JNIEnv *env,
        jobject ) {
    // todo: whelp, apparently the NDK's hello world example crashes... good old JNI.
    // Anyway, it looks like the workaround is to receive/create a jstring and then
    // call env->GetStringUTFChars(jstr, 0) to get the modified UTF 8 encoded version as char*
    // and then create a std::string over that char*.  THEN we should be able do the NewStringUTF() shuffle.
    // Alternatively, either receive the string from Java so we don't have to worry about encoding
    // or create and return a jobject that is actually a java.land.String based on the actual string data
    // we want to use.  Even that requires GetStringUTFChars() over the encoding string, though, so
    // there's not really a nice solution AFAICS
    std::string helloString = "hello strings from JNI";
    return env->NewStringUTF(helloString.c_str());
}


