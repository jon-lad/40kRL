-- Homeworlds.lua
-- Defines homeworld templates for character generation.
-- Loaded by Engine at startup via sol2.
--
-- Each entry has:
--   name           : string  (display name)
--   description    : string  (lore text shown in chargen menu)
--   charMods       : table   (ws, bs, s, t, ag, int, per, wp, fel as signed integers)
--   startingSkills : array   (skill name strings granted at Trained rank)
--   startingTraits : array   (trait name strings recorded on character)

homeworlds = {
    {
        name = "Void Born",
        description = "Born aboard a starship or void station, you have spent your life between the stars. The endless dark has touched your soul, granting strange resilience of will but leaving your body frail from low gravity.",
        charMods = { ws = 0, bs = 0, s = -5, t = 0, ag = 0, int = 0, per = 0, wp = 5, fel = 0 },
        startingSkills = { "Navigation (Stellar)", "Pilot (Spacecraft)" },
        startingTraits = { "Void Accustomed", "Charmed" },
    },
    {
        name = "Hive World",
        description = "Raised in the teeming hab-blocks of a hive city, you learned early to be quick on your feet and quicker with your tongue. The endless crowds taught you awareness, but trust is a luxury you never learned.",
        charMods = { ws = 0, bs = 0, s = 0, t = 0, ag = 5, int = 0, per = 0, wp = 0, fel = -5 },
        startingSkills = { "Awareness", "Deceive" },
        startingTraits = { "Hivebound", "Wary" },
    },
    {
        name = "Feral World",
        description = "You hail from a savage world where strength and cunning determine survival. Civilisation is an alien concept; your muscles and instincts are your greatest weapons, though refined learning eludes you.",
        charMods = { ws = 5, bs = 0, s = 5, t = 0, ag = 0, int = -5, per = 0, wp = 0, fel = -5 },
        startingSkills = { "Survival", "Tracking" },
        startingTraits = { "Iron Stomach", "Primitive" },
    },
    {
        name = "Death World",
        description = "Every creature, plant, and weather pattern on your homeworld was a lethal threat. Only the toughest and most vigilant survive to adulthood. You are hard as ceramite, though social graces were never a priority.",
        charMods = { ws = 5, bs = 0, s = 0, t = 5, ag = -5, int = 0, per = 0, wp = 0, fel = -5 },
        startingSkills = { "Survival", "Tracking" },
        startingTraits = { "Iron Stomach", "Paranoid" },
    },
    {
        name = "Forge World",
        description = "Born into the domain of the Adeptus Mechanicus, you were raised among the sacred machines. Your mind is sharp and your will strong, but the weakness of flesh was never adequately addressed.",
        charMods = { ws = 0, bs = 0, s = -5, t = 0, ag = 0, int = 5, per = 0, wp = 5, fel = -5 },
        startingSkills = { "Tech-Use", "Logic" },
        startingTraits = { "Mechanicus Implants", "Cog Boy" },
    },
    {
        name = "Noble Born",
        description = "You were raised in privilege among the ruling elite of your world. Fine manners, courtly intrigue, and the weight of expectation shaped you. Your charm is undeniable, though your body and will are untested.",
        charMods = { ws = 0, bs = 0, s = 0, t = -5, ag = 0, int = 0, per = 0, wp = -5, fel = 10 },
        startingSkills = { "Charm", "Command" },
        startingTraits = { "Supremely Connected", "Vendetta" },
    },
    {
        name = "Imperial World",
        description = "A standard world of the Imperium, steeped in the liturgy and doctrine of the God-Emperor. You are pious and observant, with a keen eye for detail, though sheltered from the wider horrors of the galaxy.",
        charMods = { ws = 0, bs = 0, s = -5, t = 0, ag = -5, int = 0, per = 5, wp = 5, fel = 0 },
        startingSkills = { "Common Lore (Imperium)", "Literacy" },
        startingTraits = { "Blessed Ignorance", "Liturgical Familiarity" },
    },
    {
        name = "Frontier World",
        description = "Your world sits at the edge of known space, barely tamed. Self-reliance and sharp senses are essential when Imperial law is a distant memory and the next settlement is days of travel away.",
        charMods = { ws = 0, bs = 5, s = 0, t = 0, ag = 0, int = -5, per = 5, wp = 0, fel = -5 },
        startingSkills = { "Survival", "Navigation (Surface)" },
        startingTraits = { "Frontier Hardiness", "Wilderness Savvy" },
    },
}
