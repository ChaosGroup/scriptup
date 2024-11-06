require 'fileutils'
require 'net/http'
require 'uri'
require 'zip'
require 'digest'
require 'tmpdir'

class Package

  def initialize(tag:, file:, sha:)
    @tag = tag
    @file = file
    @sha = sha
  end

  def download(path)
    Dir.mktmpdir { |tmpdir|
      zip = fetch(tmpdir)
      unzip(zip, path)
    }
  end

  private

  def fetch(directory)
    response = fetch_with_redirect("https://github.com/ChaosGroup/scriptup/releases/download/#{@tag}/#{@file}")
    
    package = "#{directory}/#{@file}"
    File.open(package, 'wb') { |f| f.write(response.body) }
    sha = Digest::SHA256.file(package).hexdigest
    return package if sha == @sha

    raise "SDK Package (#{package}) sha (#{sha}) does match expected sha: #{@sha}"
  end

  def fetch_with_redirect(uri_str, limit = 10)
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
  def unzip(zip_file_path, output)
    Zip::File.open(zip_file_path) do |zip_file|
      zip_file.each do |entry|
        destination_path = "#{output}/#{entry.name}"
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
