/*
 * ResetButton.h
 *  Monitors for the "flash" button being held down.
 *  Resets all settings expect for Prefix Name
 * 
 * Author: Ken Reneris <https://github.com/KenReneris>
 * MIT License
 * ----------------------------------------------------------
 */


class ResetButton
{
public:
    void            Loop( bool pressed );
    static void     FullReset( bool preserveName );

private:
    uint32          _pressed;
};
