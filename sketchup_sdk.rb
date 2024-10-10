require 'fileutils'
require 'net/http'
require 'uri'
require 'zip'
require 'digest'

class SketchUpSDK

  PACKAGES_INFO = {
    mac: {
      name: 'sketchup-sdk-mac',
      tag: 'v0.0.0',
      file: 'SDK_Mac_2024-0-595.zip',
      sha: '41968e873fcf74ead4eee95faa22dc8148e08dad7442fdab3d294cbb724cc2e3'
    },
    win: {
      name: 'sketchup-sdk-win',
      tag: 'v0.0.0',
      file: 'SDK_WIN_x64_2024-0-594.zip',
      sha: 'aa16603d53b75d805ad05cf4cd748f3b0e97076100aba442a034697e54a95b3e'
    }
  }.freeze

  def self.download(platform, path)
    package_info = PACKAGES_INFO[platform]
    package = "#{path}/#{package_info[:file]}"
    response = fetch_with_redirect("https://github.com/ChaosGroup/scriptup/releases/download/#{package_info[:tag]}/#{package_info[:file]}")

    File.open(package, 'wb') { |f| f.write(response.body) }
    sha = Digest::SHA256.file(package).hexdigest
    return package if sha == package_info[:sha]

    # FileUtils.rm(package)
    raise "SDK Package (#{package}) sha (#{sha}) does match expected sha: #{package_info[:sha]}"
  end

  def self.fetch_with_redirect(uri_str, limit = 10)
    raise 'Too many redirects' if limit.zero?

    uri = URI(uri_str)
    request = Net::HTTP::Get.new(uri)
    response = Net::HTTP.start(uri.hostname, uri.port, use_ssl: true) { |http| http.request(request) }

    case response
    when Net::HTTPSuccess then response
    when Net::HTTPRedirection then fetch_with_redirect(response['location'], limit - 1)
    else
      raise "Failed to fetch package: #{response.code} #{response.message}"
    end
  end

  # rubocop:disable Metrics/MethodLength
  def self.unzip(zip_file_path, output)
    Zip::File.open(zip_file_path) do |zip_file|
      zip_file.each do |entry|
        destination_path = "#{output}/#{entry.name}"
        next unless entry.name.start_with?('binaries/sketchup', 'headers/SketchUpAPI', 'SketchUpAPI.framework')

        if entry.ftype == :symlink
          target = entry.get_input_stream.read # Symlink target is stored as file content
          File.symlink(target, destination_path)
        elsif entry.directory?
          FileUtils.mkdir_p(destination_path)
        else
          FileUtils.mkdir_p(File.dirname(destination_path))
          entry.extract(destination_path) { true } # Overwrite existing files
        end
      end
    end
  end
  # rubocop:enable Metrics/MethodLength

end
