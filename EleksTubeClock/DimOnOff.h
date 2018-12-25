/*
 * DimOnOff.h
 *  Tracks when the clock should be dimmed.
 *
 * Author: Ken Reneris <https://github.com/KenReneris>
 * MIT License
 * ----------------------------------------------------------
 */


class DimOnOff : public OnOff
{
public:
    void        Loop();
    void        NextFrame();

    bool        IsDim() const;

private:
    uint8       _target;            // the target brightness
};
