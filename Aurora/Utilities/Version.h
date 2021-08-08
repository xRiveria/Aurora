namespace Aurora
{
    // Main engine core, major updates.
    int GetVersionMajor();

    // Minor features, compatibility breaking changes.
    int GetVersionMinor();

    // Minor bug fixes, alterations, refactors, updates.
    int GetVersionRevision();

    const char* GetVersionString();
}