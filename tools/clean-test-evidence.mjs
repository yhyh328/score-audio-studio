import { rm } from "node:fs/promises";

const evidenceDirectory = new URL(
    "../docs/test-evidence/",
    import.meta.url
)

await rm(evidenceDirectory, {
    recursive: true,
    force: true
});

console.log("Removed docs/test-evidence.");