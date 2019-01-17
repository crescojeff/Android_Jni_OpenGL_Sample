#include <jni.h>
#include <string>
#include <stdlib.h>

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
extern "C" JNIEXPORT jstring JNICALL
Java_com_jeffcreswell_jniopengl_jni_JniHooks_randomStringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    // todo: choose randomly from adj1, adj2, and animal corpi and then combine for the result string

    // random value from 0 to the size of our adjective1 corpus, which is calculated
    // as the entire size of the array divided by the size of String object
    int adj1Index = rand() % (sizeof(adjective1Corpus) / sizeof(std::string));
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}
