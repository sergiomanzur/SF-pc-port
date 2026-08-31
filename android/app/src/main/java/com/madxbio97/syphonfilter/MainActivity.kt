package com.madxbio97.syphonfilter

import android.app.AlertDialog
import android.content.Intent
import android.net.Uri
import android.os.Bundle
import android.widget.Toast
import androidx.activity.result.contract.ActivityResultContracts
import org.libsdl.app.SDLActivity
import java.io.File
import java.io.FileOutputStream

class MainActivity : SDLActivity() {

    private var selectedCuePath: String? = null

    private val selectCueLauncher = registerForActivityResult(ActivityResultContracts.OpenDocument()) { uri: Uri? ->
        if (uri != null) {
            val path = copyUriToInternalStorage(uri)
            selectedCuePath = path
            saveGameImagePath(path)
            showLaunchDialog()
        }
    }

    override fun getMainSharedObject(): String {
        return "libsyphon_filter.so"
    }

    override fun getMainLibraries(): Array<String> {
        return arrayOf("syphon_filter")
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        extractAssetsIfNeeded()

        val savedImage = loadSavedGameImagePath()
        if (savedImage != null && File(savedImage).exists()) {
            selectedCuePath = savedImage
        }
    }

    override fun getArguments(): Array<String> {
        val args = mutableListOf<String>()
        val cue = selectedCuePath ?: loadSavedGameImagePath()
        if (cue != null) {
            args.add("--game")
            args.add(cue)
        }
        return args.toTypedArray()
    }

    private fun showLaunchDialog() {
        val options = arrayOf(
            getString(R.string.launch_game),
            getString(R.string.select_disc),
            getString(R.string.graphics_settings),
            getString(R.string.dossiers)
        )

        AlertDialog.Builder(this)
            .setTitle(R.string.app_name)
            .setItems(options) { _, which ->
                when (which) {
                    0 -> {
                        if (selectedCuePath == null) {
                            Toast.makeText(this, R.string.select_disc, Toast.LENGTH_LONG).show()
                            selectCueLauncher.launch(arrayOf("*/*"))
                        }
                    }
                    1 -> selectCueLauncher.launch(arrayOf("*/*"))
                    2 -> startActivity(Intent(this, SettingsActivity::class.java))
                    3 -> startActivity(Intent(this, DossierActivity::class.java))
                }
            }
            .setCancelable(false)
            .show()
    }

    private fun extractAssetsIfNeeded() {
        val localesDir = File(filesDir, "locales/ru-vit")
        val skyboxDir = File(filesDir, "assets/skyboxes")
        if (!localesDir.exists()) localesDir.mkdirs()
        if (!skyboxDir.exists()) skyboxDir.mkdirs()

        copyAssetFolder("locales", File(filesDir, "locales"))
        copyAssetFolder("skyboxes", File(filesDir, "assets/skyboxes"))
    }

    private fun copyAssetFolder(assetPath: String, targetDir: File) {
        try {
            val list = assets.list(assetPath) ?: return
            if (list.isEmpty()) {
                // It's a file
                val targetFile = File(targetDir, File(assetPath).name)
                if (!targetFile.exists()) {
                    assets.open(assetPath).use { input ->
                        FileOutputStream(targetFile).use { output ->
                            input.copyTo(output)
                        }
                    }
                }
            } else {
                targetDir.mkdirs()
                for (file in list) {
                    copyAssetFolder("/", File(targetDir, file))
                }
            }
        } catch (e: Exception) {
            e.printStackTrace()
        }
    }

    private fun copyUriToInternalStorage(uri: Uri): String {
        val fileName = "game.cue"
        val file = File(filesDir, fileName)
        contentResolver.openInputStream(uri)?.use { input ->
            FileOutputStream(file).use { output ->
                input.copyTo(output)
            }
        }
        return file.absolutePath
    }

    private fun saveGameImagePath(path: String) {
        val dir = File(filesDir, "SyphonFilterPC")
        if (!dir.exists()) dir.mkdirs()
        val ini = File(dir, "launcher.ini")
        val text = if (ini.exists()) ini.readText() else ""
        ini.writeText(text + "\n[Game]\nImage=\n")
    }

    private fun loadSavedGameImagePath(): String? {
        val ini = File(File(filesDir, "SyphonFilterPC"), "launcher.ini")
        if (!ini.exists()) return null
        for (line in ini.readLines()) {
            if (line.startsWith("Image=")) {
                return line.substring("Image=".length).trim()
            }
        }
        return null
    }
}
